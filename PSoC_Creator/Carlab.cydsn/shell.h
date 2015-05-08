/* ========================================
 * shell.h
 * Victor A. Ying and Monica Lu
 *
 * a very minimal shell.
 * ========================================
 */

#ifndef SHELL_H
#define SHELL_H

#define SHELL_MAX_COMMAND_LENGTH 63

/*
#define SHELL_LCD_DEBUG // Uncomment this line to print keypresses to the LCD
*/

/*
 * shell_do_command()
 * A function for parsing and dispatching command lines.
 * Must be passed a null-terminated string!
 */
void shell_do_command(const char* line) CYREENTRANT ;

/*
 * shell_handle_char()
 * For every character that has been received, calls handler with
 * that character. Does nothing if no data has been received since
 * the previous time this function was called.
 */
void shell_handle_char(char c) ;

/*
 * shell_handle_recieved_chars()
 * For every character that has been received, calls handler with that character.
 * Does nothing if no data has been received since the previous time this function
 * was called.
 */
void shell_handle_received_chars(void) ;

#endif 
 
//[] END OF FILE
