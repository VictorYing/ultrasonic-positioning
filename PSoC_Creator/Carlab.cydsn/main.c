/* ========================================
 * main.c
 * Victor A. Ying and Monica Lu
 *
 * This is the main program. It contains very little code.
 * ========================================
 */

#include <project.h>
#include <stdio.h>

#include "usb_uart.h"
#include "shell.h"
#include "drive.h"
#include "speed.h"
#include "steer.h"
#include "position.h"

/*
 * MAIN PROGRAM
 */
int main(void) {
    // Turn on interrupts 
    CyGlobalIntEnable; 
    
    // Initialize USB serial connection for I/O
//    usb_uart_init(USBUART_5V_OPERATION); //!!NOTE!! Make sure this matches your board voltage!
    
    // Initialize LCD display
    LCD_Start();
    
    // Initialize radio communincation
    UART_Start();
    
    // Begin positioning
    position_init();
    
    // Initialize navigation stuff
    drive_init();
    
    // Main loop performs user interface/communication actions.
    // Other actions are performed in interrupt handlers.
    for (;;) {
//        shell_handle_received_chars();
//        speed_display_info();

        // Display position to LCD
        if (position_data_available()) {
            char buf[32], radiobuf[32];
            float x, y;
            uint16 counter = 0u;
            uint8 status = CyEnterCriticalSection();
            x = position_x();
            y = position_y();
            sprintf(radiobuf, "X%.2fY%.2f\n", x, y);
            UART_PutString(radiobuf);
            /*
            LCD_Position(0,0);
            LCD_PrintNumber(counter++);
            sprintf(buf, "Error:%.2f   ", error());
            LCD_Position(0,4);
            LCD_PrintString(buf);
            sprintf(buf, "X:%.2f Y:%.2f    ", x, y);
            LCD_Position(1,0);
            LCD_PrintString(buf);
            */
            CyExitCriticalSection(status);
        }        
    }
}

/* [] END OF FILE */
