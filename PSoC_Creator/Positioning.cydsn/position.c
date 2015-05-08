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
#include <stdio.h>

#include "position.h"


/*
 * CONSTANTS
 */

#define X 23.5  // distance between first and second transmitters in feet
#define Y 33.75  // distance between second and third transmitters in feet
#define Z 7.583
#define CLOCK_FREQ 1000000  // Hz
#define WAVE_SPEED 1135.0  // ft/s
#define TX_SPACING 100  // ms
#define EPSILON 0.5  // ft
#define DEL_FACTOR 0.1  // ft
#define MAX_ERROR 0.5  // ft^2
#define ERROR_THRESHOLD 0.01  // ft^2
#define MAX_ITERATIONS 50

//#define SHOW_GARBAGE  // Uncomment this to check if sanity checks are failing
//#define VERBOSE

/*
 * STATIC FUNCTION PROTOTYPES
 */

static CY_ISR_PROTO(positioningHandler) ;


/*
 * GLOBAL VARIABLES
 */

static float x = 0.0, y = 0.0;  // the current position
static float fxy = 0.0; // the current error
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
    GlitchCounter_Start();
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
    uint8 status = CyEnterCriticalSection();
    uint8 ret = new_data;
    new_data = 0u;
    CyExitCriticalSection(status);
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

float error(void) {
    return fxy;
}

float fabsf(float num) {
    if (num >= 0.0)
        return num;
    else
        return -num;
}

/*
 * positioningHandler:
 * Interrupt handler run after sequence of four pings, calculating position.
 */
static CY_ISR(positioningHandler) {
    uint32 time[4];
    float diff[4];
    int i, iters = 0;
    float new_x, new_y;
    char buf[16];

    // Get the times of arrival
    for (i = 0; i < 4; i++) {
        time[i] = UltraTimer_ReadCapture();

        // If more than a second since the last reset, then throw away this
        // set of measurements
        if (time[i] == 0u || time[i] < ULONG_MAX - CLOCK_FREQ) {
#ifdef SHOW_GARBAGE
            x = (float)i;
            y = (float)time[i];
            new_data = 1u;
#endif
            return;
        }
    }

    // Calculate differences in distances in feet
    for (i = 1; i < 4; i++) {
        diff[i] = (float)((int32)(time[0] - time[i])
                          - i*(CLOCK_FREQ/1000*TX_SPACING))
                  * (WAVE_SPEED/CLOCK_FREQ);
        
        // If difference is much larger than the size of the rectangle of
        // transmitter stations, the data is probably bad, so throw it away
        if (fabsf(diff[i]) > X + Y) {
        
#ifdef SHOW_GARBAGE
            x = (float)i;
            y = diff[i];
            new_data = 1u;
#endif
            return;
        }
    }
    
    // Positioning using Newton's method
    new_x = x;
    new_y = y;
    
    do {
        float dfx, dfy, dfxAbs, dfyAbs;
        float dist[4], error[4];
        
        // Calculate what the distances should be based on our most recent (x,y)
        dist[0] = sqrt((new_x+X/2)*(new_x+X/2) + (new_y+Y/2)*(new_y+Y/2) + Z*Z);
        dist[1] = sqrt((new_x-X/2)*(new_x-X/2) + (new_y+Y/2)*(new_y+Y/2) + Z*Z);
        dist[2] = sqrt((new_x-X/2)*(new_x-X/2) + (new_y-Y/2)*(new_y-Y/2) + Z*Z);
        dist[3] = sqrt((new_x+X/2)*(new_x+X/2) + (new_y-Y/2)*(new_y-Y/2) + Z*Z);
       
        // Calculate disagreement between hypothetical distances and measurements
        for (i = 1; i < 4; i++)
            error[i] = (dist[i]-dist[0]) - diff[i];
            
        // Calculate our metric as the sum of the squares of the errors
        fxy = 0.0;
        for (i = 1; i < 4; i++)
            fxy += error[i]*error[i];
        
        // Calculate the partial derivatives of the metric
        dfx = 2*error[2] * (((new_x - X/2)/dist[2]) - (new_x + X/2)/dist[0]);
        dfx += 2*error[3] * (((new_x + X/2)/dist[3]) - (new_x + X/2)/dist[0]);
        dfx += 2*error[1] * (((new_x - X/2)/dist[1]) - (new_x + X/2)/dist[0]);
       
        dfy = 2*error[2] * (((new_y - Y/2)/dist[2]) - (new_y + Y/2)/dist[0]);
        dfy += 2*error[3] * (((new_y - Y/2)/dist[3]) - (new_y + Y/2)/dist[0]);
        dfy += 2*error[1] * (((new_y + Y/2)/dist[1]) - (new_y + Y/2)/dist[0]);
        
        dfxAbs = fabsf(dfx);
        dfyAbs = fabsf(dfy);
        
        if (dfxAbs > dfyAbs) {
            if (dfxAbs > 0.0) {
                new_x -= DEL_FACTOR*dfx;
            }
            else {
                new_x += DEL_FACTOR;
            }
        }
        else {
            if (dfyAbs > 0.0) {
                new_y -= DEL_FACTOR*dfy;
            }
            else {
                new_y += DEL_FACTOR;
            }
        }
       
#ifdef VERBOSE 
        sprintf(buf, "dX:%.1f dY:%.1f %d  ", dfxAbs, dfyAbs, iters);
        LCD_Position(1,0);
        LCD_PrintString(buf);
        sprintf(buf, "X:%.1f Y:%.1f   ", new_x, new_y);
        LCD_Position(0,0);
        LCD_PrintString(buf);
        sprintf(buf, "%.1f     ", fxy);
        LCD_Position(0,13);
        LCD_PrintString(buf);
#endif

        iters++;
    } while ((fabsf(fxy) > ERROR_THRESHOLD) && (iters < MAX_ITERATIONS));  
    
    if (fabsf(fxy) < MAX_ERROR) {
        x = new_x;
        y = new_y;
        new_data = 1u;
    }

    // Clear interrupt
    UltraTimer_ReadStatusRegister();
}


/* [] END OF FILE */
