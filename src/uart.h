#define BAUD 9600
#define UBRR_VALUE (((F_CPU / (BAUD * 16UL))) - 1)

#include <util/setbaud.h>

void UART_init();
void UART_puti(const int val);
void UART_putc(unsigned char data);
unsigned char UART_getc (void);
void UART_puts(char *str);
