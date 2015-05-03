/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include<project.h>
#include<stdio.h>

#include "position.h"


#define X 60.0  // ft
#define Y 30.0  // ft
#define ultraClockSpeed 10000.0  // Hz
#define wavespeed 1135.0  // ft/s
#define CONVERSION_FACTOR ((wavespeed/ultraClockSpeed)*(wavespeed/ultraClockSpeed))
#define TX_SPACING 1000  // clock cycles (tenths of milliseconds)


CY_ISR(interPing) {
    uint16 pTime[4];
    float diff[3];
    float x, y;
    char buf[16];
    int i;
    
    for (i = 0u; i < 4u; i++)
        pTime[i] = UltraTimer_ReadCapture();

    for (i = 0u; i < 3u; i++)
        diff[i] = (float)((int16)(pTime[0u] - pTime[i+1u]) - (i+1)*TX_SPACING);
    
    if ((diff[2] == 0) && (diff[0] == diff[1])) {
        x = ((diff[0] * diff[0])*CONVERSION_FACTOR + X*X)/(2.0*X);
        y = 0.5 * Y;
    }
    else if ((diff[0] == 0) && (diff[1] == diff[2])) {
        x = 0.5 * X;
        y = ((diff[1] * diff[1])*CONVERSION_FACTOR + Y*Y)/(2.0*Y);        
    }
    else {
        x = (((diff[0]*(diff[0]-diff[2]-diff[1])*(diff[1]-diff[2]))
              / (diff[0]-diff[1]+diff[2]))
             * CONVERSION_FACTOR + X*X) / (2.0*X);
        y = (((diff[2]*(diff[2]-diff[0]-diff[1])*(diff[1]-diff[0]))
              / (diff[0]-diff[1]+diff[2]))
             * CONVERSION_FACTOR + Y*Y) / (2.0*Y);
    }
    
    sprintf(buf, "%2.1f", diff[0]);
    LCD_Position(0,0);
    LCD_PrintString(buf);
    sprintf(buf, "%2.1f", diff[1]);
    LCD_Position(0,9);
    LCD_PrintString(buf);
    sprintf(buf, "%2.4f", x);
    LCD_Position(1,0);
    LCD_PrintString(buf);
    sprintf(buf, "%2.4f", y);
    LCD_Position(1,9);
    LCD_PrintString(buf);

    UltraTimer_ReadStatusRegister();    
}

void position_init(void) {
    UltraCounter_Start();
    UltraTimer_Start();
    UltraComp_Start();
    UltraDAC_Start();
    Ping_inter_Start();
    Ping_inter_SetVector(interPing);   
}


/* [] END OF FILE */
