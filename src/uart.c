#include <uart.h>
#include <avr/io.h>
#include <stdint.h>


// ** Taken from Tutorial 9  **

void uart_init(void) {
    PORTB.DIRSET = PIN2_bm; // Enable PB2 as output (USART0 TXD)
    USART0.BAUD = 1389;     // 9600 baud @ 3.3 MHz
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;   // Enable Tx/Rx
}

uint8_t uart_getc(void) {
    while (!(USART0.STATUS & USART_RXCIF_bm));  // Wait for data
    return USART0.RXDATAL;
}

void uart_putc(uint8_t c) {
    while (!(USART0.STATUS & USART_DREIF_bm));  // Wait for TXDATA empty
    USART0.TXDATAL = c;
}

void uart_puts(char* string){
    while (!(USART0.STATUS & USART_DREIF_bm));  // Wait for TXDATA empty
    uint8_t *s = (uint8_t*) string;
    while (*s){
        uart_putc(*(s++));
    }
}
