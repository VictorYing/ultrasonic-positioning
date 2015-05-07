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

static CY_ISR(rxHandler) {
    char8 c;
    
    UART_ReadRxStatus();
    c = UART_GetChar();
    UART_PutChar(c);
    LCD_Position(0,0);
    LCD_PutChar(c);
}

int main(void) {
    
    int i;
    // Enable interrupts
    CyGlobalIntEnable;
    
    RxIRQ_Start();
    RxIRQ_SetVector(rxHandler);

    // Prepare for printing to the LCD
    LCD_Start();
    UART_Start();

    // Begin positioning
    position_init();

    // Do UI stuff in this loop
    for(i = 0; i < 10; i++) {
        char buf[16];
        char8 c;

#ifndef VERBOSE
        // Display position to LCD
        if (position_data_available()) {
            
            static uint16 counter = 0u;
            LCD_Position(0,0);
            LCD_PrintNumber(counter++);
            sprintf(buf, "X:%.2f Y:%.2f    ", position_x(), position_y());
            LCD_Position(1,0);
            LCD_PrintString(buf);
            sprintf(buf, "X:%.2f Y:%.2f \n\r", position_x(), position_y());
            UART_PutString(buf);
        }
#endif
        sprintf(buf, "X:%.2f Y:%.2f \n\r", position_x(), position_y());
        UART_PutString(buf);    
        
    }
}

/* [] END OF FILE */
