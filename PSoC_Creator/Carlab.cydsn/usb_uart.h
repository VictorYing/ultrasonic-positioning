/* ========================================
 * usbuart_helpers.h
 * Victor A. Ying and Monica Lu
 *
 * Functions to manage the serial
 * connection over USB.
 * ========================================
 */

#ifndef USB_UART_H
#define USB_UART_H

#include "USBUART.h"
#include "shell.h"

/*
 * usb_uart_init()
 * Initialize USBUART (and possibly LCD) for I/O
 * NOTE: Make sure mode matches your board voltage!
 */
void usb_uart_init(uint8 mode);
 
/*
 * usb_uart_print_info()
 * Sends information about the USBUART connection to the USBUART connection.
 */
void usb_uart_print_info(void);
 
/*
 * usb_uart_put*()
 * Wrappers around Cypress's provided USBUART_Put*() functions
 * that wait for previous Tx to finish before continuing.
 */
void usb_uart_putdata(const uint8* pData, uint16 length);
void usb_uart_putstring(const char8* string);
void usb_uart_putchar(char8 txDataByte);
void usb_uart_putCRLF(void);
 
/*
 * usb_uart_putline()
 * An additional helpful function for printing a string followed by CRLF.
 */
void usb_uart_putline(const char8* string);


#endif
 
//[] END OF FILE
