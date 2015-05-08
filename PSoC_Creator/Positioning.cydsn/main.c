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

//#define VERBOSE // Uncomment this line to display Newton Method output

int main(void) {
    

    // Enable interrupts
    CyGlobalIntEnable;

    // Prepare for printing to the LCD
    LCD_Start();
    UART_Start();

    // Begin positioning
    position_init();

    // Do UI stuff in this loop
    for(;;) {
        char buf[80];
        float x, y;

        // Display position to LCD
        if (position_data_available()) {
            static uint16 counter = 0u;
            x = position_x();
            y = position_y();
            sprintf(buf, "X%.2fY%.2f\n", x, y);
            UART_PutString(buf);    
            
            LCD_Position(0,0);
            LCD_PrintNumber(counter++);
            sprintf(buf, "Error:%.2f   ", error());
            LCD_Position(0,4);
            LCD_PrintString(buf);
            sprintf(buf, "X:%.2f Y:%.2f    ", x, y);
            LCD_Position(1,0);
            LCD_PrintString(buf);
        }        
    }
}

/* [] END OF FILE */
