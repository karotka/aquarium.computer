#include <avr/io.h>
#include "uart.h"
#include <stdlib.h>


void UART_init() {
    UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
    UBRR0L = (uint8_t)UBRR_VALUE;

    // Set
    //UCSR0B = (1 << RXEN0);    // Enable receiver
    UCSR0B = (1 << TXEN0);    // Enable transmiter
    UCSR0C = (1 << UCSZ00) | (1 << UCSZ01) | (1 << USBS0);  // set to 8 data bits, 2 stop bit
    //UCSR0B |= (1 << RXCIE0);
}

void UART_puti(const int val){
    char buffer[8];
    UART_puts(itoa(val, buffer, 10));
}

void UART_putc(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));       // Wait for empty transmit buffer
    UDR0 = data;                           // Start transmittion
}

unsigned char UART_getc (void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void UART_puts(char *str) {
    while(*str) {
        UART_putc(*str++);
    }
}
