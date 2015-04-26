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
#define CLOCK_FREQ 10000  // Hz
#define WAVE_SPEED 1135.0  // ft/s
#define TX_SPACING 1000  // clock cycles (tenths of milliseconds)
#define EPSILON 0.1  // ft


/*
 * STATIC FUNCTION PROTOTYPES
 */

static CY_ISR_PROTO(positioningHandler) ;


/*
 * GLOBAL VARIABLES
 */

static float x = 0.0, y = 0.0;  // the current position


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

    // Get the times of arrival
    for (i = 0; i < 4; i++) {
        time[i] = UltraTimer_ReadCapture();

        // If more than a second since the last reset, then throw away this
        // set of measurements
        if (time[i] < USHRT_MAX - CLOCK_FREQ)
            return;
    }

    // Calculate differences in distances in feet
    for (i = 1; i < 4; i++) {
        diff[i] = (float)((int16)(time[0] - time[i]) - i*TX_SPACING)
                  * (WAVE_SPEED/CLOCK_FREQ);

        // If difference is much larger than the size of the rectangle of
        // transmitter stations, the data is probably bad, so throw it away
        if (diff[i] > X + Y)
            return;
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

    // Clear interrupt
    UltraTimer_ReadStatusRegister();
}


/* [] END OF FILE */
