/* ========================================
 * usb_uart.c
 * Victor A. Ying and Monica Lu
 * 
 * Manage the serial connection over USB
 * ========================================
 */

#include <project.h>
#include <stdio.h>

#include "usb_uart.h"


/*
 * usb_uart_init()
 * Initialize USBUART for I/O. Should be called after CYGlobalIntEnable;.
 * NOTE: Make sure mode matches your board voltage!
 */
void usb_uart_init(uint8 mode) {
    USBUART_Start(0, mode); 
    while(!USBUART_GetConfiguration()) 
        ;
    USBUART_CDC_Init();
}

/*
 * usb_uart_print_info()
 * Sends information about the USBUART connection to the USBUART connection.
 */
void usb_uart_print_info(void) {
    char8 strbuf[128];
    
    // USBUART info
    uint32 rate = USBUART_GetDTERate(); 
    uint8 stop_bits = USBUART_GetCharFormat();
    uint8 parity = USBUART_GetParityType();
    uint8 data_bits = USBUART_GetDataBits();
    sprintf(strbuf, 
            "UART over USB data rate is %lu bits per second", rate);
    usb_uart_putline(strbuf); 
    sprintf(strbuf, 
            "UART over USB characters consist of %i data bits",
            (int)data_bits);
    usb_uart_putline(strbuf);
    sprintf(strbuf, "UART over USB parity mode; %s", 
            (parity == USBUART_PARITY_NONE) ? "none" :
            (parity == USBUART_PARITY_ODD) ? "odd" :
            (parity == USBUART_PARITY_EVEN) ? "even" :
            (parity == USBUART_PARITY_MARK) ? "mark" :
            (parity == USBUART_PARITY_SPACE) ? "space" :
            "?ERROR?");
    usb_uart_putline(strbuf);
    sprintf(strbuf, "UART over USB stop bits: %s",
            (stop_bits == USBUART_1_STOPBIT) ? "1" :
            (stop_bits == USBUART_1_5_STOPBITS) ? "1.5" :
            (stop_bits == USBUART_2_STOPBITS) ? "2" :
            "?ERROR?");
    usb_uart_putline(strbuf);
}

/*
 * usb_uart_put*()
 * Wrappers around Cypress's provided USBUART_Put*() functions
 * that wait for previous Tx to finish before continuing.
 */
void usb_uart_putdata(const uint8* pData, uint16 length)  { 
    while(!USBUART_CDCIsReady())
        ;
    USBUART_PutData((uint8 *)pData, length);
}
void usb_uart_putstring(const char8* str) {
    while(!USBUART_CDCIsReady())
        ;
    USBUART_PutString((char8 *)str);
}
void usb_uart_putchar(char8 txDataByte) {
    while(!USBUART_CDCIsReady())
        ;
    USBUART_PutChar(txDataByte);
}
void usb_uart_putCRLF(void) {
    while(!USBUART_CDCIsReady())
        ;
    USBUART_PutCRLF();
}

/*
 * usb_uart_putline()
 * An additional helpful function for printing a string followed by CRLF.
 */
void usb_uart_putline(const char8* str) {
    usb_uart_putstring(str);
    usb_uart_putCRLF();
}
    
/* [] END OF FILE */
