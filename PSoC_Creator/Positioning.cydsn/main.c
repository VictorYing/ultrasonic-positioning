/* ===========================================================
 *
 * position.h
 * Michael Danielczuk, Andrew Kim, Monica Lu, and Victor Ying
 *
 * An example client program.
 *
 * ===========================================================
 */

#include <project.h>
#include <stdio.h>
#include <math.h>

#include "position.h"


int main(void) {
    
    // Enable interrupts
    CyGlobalIntEnable;

    // Prepare for printing to the LCD
    LCD_Start();

    // Begin positioning
    position_init();

    // Do UI stuff in this loop
    for(;;) {
        char buf[16];

        // Display position to LCD
        if (position_data_available()) {
            static uint16 counter = 0u;
            LCD_Position(0,0);
            LCD_PrintNumber(counter++);
            sprintf(buf, "X:%.1f Y:%.1f    ", position_x(), position_y());
            LCD_Position(1,0);
            LCD_PrintString(buf);
        }       
    }   
}

/* [] END OF FILE */
