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
        sprintf(buf, "X:%2.2f  Y:%2.2f  ", position_x(), position_y());
        LCD_Position(1,0);
        LCD_PrintString(buf);
        CyDelay(50u);
    }
}

/* [] END OF FILE */
