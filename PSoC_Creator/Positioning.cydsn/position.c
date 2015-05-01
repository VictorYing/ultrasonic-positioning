/* ===========================================================
 *
 * position.c
 * Michael Danielczuk, Andrew Kim, Monica Lu, and Victor Ying
 *
 * Provides positioning using time difference of arrival
 * multilateration with four transmitters arranged in a
 * rectangle. Assumes the transmitters send out pings in turn
 * with a certain amount of spacing time between when pings are
 * sent, and that these pings are sent in a counterclockwise
 * order.
 *
 * ===========================================================
 */

#include <project.h>
#include <math.h>
#include <limits.h>

#include "position.h"


/*
 * CONSTANTS
 */

#define X 60.0  // distance between first and second transmitters in feet
#define Y 30.0  // distance between second and third transmitters in feet
#define CLOCK_FREQ 20000  // Hz
#define WAVE_SPEED 1135.0  // ft/s
#define TX_SPACING 100  // ms
#define EPSILON 0.5  // ft

//#define DEBUG  // Uncomment this define to check if sanity checks are failing


/*
 * STATIC FUNCTION PROTOTYPES
 */

static CY_ISR_PROTO(positioningHandler) ;


/*
 * GLOBAL VARIABLES
 */

static float x = 0.0, y = 0.0;  // the current position
static uint8 new_data = 0u;  // Boolean indicating whether new data available


/*
 * FUNCTIONS
 */

/*
 * position_init:
 * Start positioning.
 */
void position_init(void) {
    UltraCounter_Start();
    UltraTimer_Start();
    UltraComp_Start();
    UltraDAC_Start();
    UltraIRQ_Start();
    UltraIRQ_SetVector(positioningHandler);
}

/*
 * position_data_available:
 * returns nonzero if new data since the last time this function was called.
 */
uint8 position_data_available(void) {
    uint8 ret = new_data;
    new_data = 0u;
    return ret;
}

/*
 * position_?:
 * Getter functions for position in units of feet, with the origin at the
 * center of the rectangle formed by the transmitters.
 */
float position_x(void) {
    return x;
}
float position_y(void) {
    return y;
}

/*
 * positioningHandler:
 * Interrupt handler run after sequence of four pings, calculating position.
 */
static CY_ISR(positioningHandler) {
    uint16 time[4];
    float diff[4];
    int i;
    
    // Clear interrupts
    UltraTimer_ReadStatusRegister();


    // Get the times of arrival
    for (i = 0; i < 4; i++) {
        time[i] = UltraTimer_ReadCapture();

        // If more than a second since the last reset, then throw away this
        // set of measurements
        if (time[i] == 0u || time[i] < USHRT_MAX - CLOCK_FREQ) {
#ifdef DEBUG
            x = (float)i;
            y = (float)time[i];
            new_data = 1u;
#endif
            return;
        }
    }

    // Calculate differences in distances in feet
    for (i = 1; i < 4; i++) {
        diff[i] = (float)((int16)(time[0] - time[i])
                          - i*(CLOCK_FREQ/1000*TX_SPACING))
                  * (WAVE_SPEED/CLOCK_FREQ);

        // If difference is much larger than the size of the rectangle of
        // transmitter stations, the data is probably bad, so throw it away
        if (diff[i] > X + Y) {
#ifdef DEBUG
            x = (float)i;
            y = diff[i];
            new_data = 1u;
#endif
            return;
        }
    }

    // Calculate position
    if (fabs(diff[1]) < EPSILON) {
        x = 0.0;
        y = diff[3] * diff[2] / (2.0 * Y);
    }
    else if (fabs(diff[3]) < EPSILON) {
        x = diff[1] * diff[2] / (2.0 * X);
        y = 0.0;
    }
    else {
        x = diff[1] * (diff[1]-diff[3]-diff[2]) * (diff[2]-diff[3])
            / (2.0 * X * (diff[1]+diff[3]-diff[2]));
        y = diff[3] * (diff[3]-diff[1]-diff[2]) * (diff[2]-diff[1])
            / (2.0 * Y * (diff[1]+diff[3]-diff[2]));
    }
    new_data = 1u;

}


/* [] END OF FILE */
