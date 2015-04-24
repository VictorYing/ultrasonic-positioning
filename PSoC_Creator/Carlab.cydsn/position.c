/* ===========================================================
 *
 * position.c
 * Michael Danielczuk, Andrew Kim, Monica Lu, and Victor Ying
 *
 * Provides positioning using time difference of arrival
 * multilateration with four transmitters arranged in a
 * rectangle.
 *
 * ===========================================================
 */

#include<project.h>
#include<math.h>

#include "position.h"


#define X 60.0  // ft
#define Y 30.0  // ft
#define CLOCK_FREQ 10000.0  // Hz
#define WAVE_SPEED 1135.0  // ft/s
#define TX_SPACING 1000  // clock cycles (tenths of milliseconds)
#define EPSILON 0.1  // ft


static float x = 0.0, y = 0.0;

// Runs after each sequence of four pings. Calculates position.
static CY_ISR(positioningHandler) {
    uint16 time[4];
    float diff[4];
    int i;

    // Get the times of arrival
    for (i = 0; i < 4; i++)
        time[i] = UltraTimer_ReadCapture();

    // Calculate differences in distances in feet
    for (i = 1; i < 4; i++)
        diff[i] = (float)((int16)(time[0] - time[i]) - i*TX_SPACING)
                  * (WAVE_SPEED/CLOCK_FREQ);

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

    // Clear interrupt request
    UltraTimer_ReadStatusRegister();
}

// Begin positioning
void position_init(void) {
    UltraCounter_Start();
    UltraTimer_Start();
    UltraComp_Start();
    UltraDAC_Start();
    UltraIRQ_Start();
    UltraIRQ_SetVector(positioningHandler);
}

// Getter functions for position in units of feet
float position_x(void) {
    return x;
}
float position_y(void) {
    return y;
}

/* [] END OF FILE */
