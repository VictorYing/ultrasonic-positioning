/* ========================================
 * shell.c
 * Victor A. Ying and Monica Lu
 *
 * a very minimal shell.
 * ========================================
 */

#include <project.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "speed.h"
#include "steer.h"
#include "usb_uart.h"
#include "drive.h"

/*
 * vshell_do_command()
 * A function for parsing and dispatching command lines.
 * Must be passed a null-terminated string!
 */
void shell_do_command(const char* line) CYREENTRANT {
    char cmd[SHELL_MAX_COMMAND_LENGTH + 1];
    uint8 i = 0u;
    
    // Ignore leading whitespace
    while (isspace(*line))
        line++;
        
    // Identify command as the longest prefix not containing a space.
    while (*line != '\0' && !isspace(*line)) {
        cmd[i] = *line;
        i++;
        line++;
    }
    cmd[i] = '\0';

    // Throw away any whitespace between the command and the arguments
    while (isspace(*line))
        line++;
    
    // Now we can execute the identified command
    
    // Do nothing in case of empty command
    if (strcmp(cmd, "") == 0) 
        ;
    
    // Shell built-in commands
    else if (strcmp(cmd, "repeat") == 0) {
        char numloops[SHELL_MAX_COMMAND_LENGTH];
        i = 0u;
        
        while (*line != '\0' && !isspace(*line)) {
            numloops[i] = *line;
            i++;
            line++;
        }
        numloops[i] = '\0';
        while (isspace(*line))
            line++;
        
        for (i = 0; i < atoi(numloops); i++) {
            shell_do_command(line);
        }
    }
    
    // Other commands
    else if (strcmp(cmd, "serial") ==0) {
        usb_uart_print_info();
    }
    else if (strcmp(cmd, "lcd") == 0) {
        LCD_ClearDisplay();
        LCD_PrintString(line);
    }
    else if (strcmp(cmd, "pid") == 0) {
        speed_pid_start(line);
    }
    else if (strcmp(cmd, "brake") == 0) {
        speed_brake();
    }
    else if (strcmp(cmd, "coast") == 0) {
        speed_coast();
    }
    else if (strcmp(cmd, "fw") == 0) {
        speed_forward();
    }
    else if (strcmp(cmd, "bw") == 0) {
        speed_backward();
    }
    else if (strcmp(cmd, "power") == 0) {
        speed_set_power(line);
    }
    else if (strcmp(cmd, "speedkp") == 0) {
        speed_set_kp(line);
    }
    else if (strcmp(cmd, "speedki") == 0) {
        speed_set_ki(line);
    }
    else if (strcmp(cmd, "speedkd") == 0) {
        speed_set_kd(line);
    }
    else if (strcmp(cmd, "steerpid") == 0) {
        steer_pid_start();
    }
    else if (strcmp(cmd, "steerstop") == 0) {
        steer_stop();
    }
    else if (strcmp(cmd, "steerset") == 0) {
        steer_set(line);
    }
    else if (strcmp(cmd, "steerkp") == 0) {
        steer_set_kp(line);
    }
    else if (strcmp(cmd, "steerki") == 0) {
        steer_set_ki(line);
    }
    else if (strcmp(cmd, "steerkd") == 0) {
        steer_set_kd(line);
    }
    // If command was not any of the above...
    else {
        char8 strbuf[128];
        
        sprintf(strbuf, "Command \"%s\" not recognized", cmd);
        usb_uart_putline(strbuf);
    }
}

/*
 * shell_handle_char()
 * A function for handling keypresses from a
 * terminal, and building up command lines.
 */
void shell_handle_char(char c) {
    static char linebuf[SHELL_MAX_COMMAND_LENGTH + 1];
    static uint8 line_index = 0;
    
    if (c < 127 && c >= 32) { // If the user pressed a character 
        #ifdef SHELL_LCD_DEBUG
        // For debugging purposes, display the character to the LCD
        LCD_ClearDisplay();
        LCD_PutChar(c);
        #endif
        
        if (line_index < SHELL_MAX_COMMAND_LENGTH) {
            // Add character to current line
            linebuf[line_index] = c;
            line_index++;
            
            // Echo character to terminal.
            usb_uart_putchar(c);
        }
    }
    else if (c == '\b' || c == '\177') { // If the user pressed backspace
        #ifdef SHELL_LCD_DEBUG
        // For debugging purposes, display to the LCD
        LCD_ClearDisplay();
        LCD_PrintString("Backspace");
        #endif
        
        if (line_index > 0) {
            // Clear one character on the terminal
            usb_uart_putstring("\b \b");
            
            // Discard one character from the command buffer
            line_index--;
        }
    }
    else if (c == '\r') { // If the user has hit the Return/Enter key
        #ifdef VSHELL_LCD_DEBUG
        // For debugging purposes, display to the LCD
        LCD_ClearDisplay();
        LCD_PrintString("Return");
        #endif
        
        // Echo newline to terminal
        usb_uart_putCRLF();
        
        // Do command
        linebuf[line_index] = '\0';
        shell_do_command(linebuf);
        
        // Print a prompt to indicate ready for next command
        usb_uart_putstring("$ ");
        line_index = 0;
    }
    else { // For any other characters/keypress
        #ifdef SHELL_LCD_DEBUG
        char8 strbuf[128];
        
        // For aid in debugging, print out ASCII codes in hex
        sprintf(strbuf, " %X", (int)c);
        LCD_PrintString(strbuf);
        #endif
    }
}

/*
 * shell_handle_recieved_chars()
 * For every character that has been received, calls handler with
 * that character. Does nothing if no data has been received since
 * the previous time this function was called.
 */
void shell_handle_received_chars(void) {
    while(USBUART_DataIsReady()) {  // Check for input data from PC
        uint8 i, count, buffer[128];
        
        count = USBUART_GetData(buffer, sizeof(buffer));  // Get any data from PC
        for (i = 0; i < count; i++) {  // For each character recieved
            shell_handle_char(buffer[i]); // Pass the character to the handler
        }
    }
}
 
/* [] END OF FILE */
