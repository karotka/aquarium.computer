#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
//#include "uart.h"


void portConfig(void) {
    // Port c as output
    DDRB  = 0xff;
    PORTB = 0x00;

    // Port b 1, 0 as input
    DDRD  = 0xff;
    PORTD = 0x00;
}

void timer0start(void) {
    TCCR0B |= (1 << CS01) | (1 << CS00);   // Prescaler = FCPU /256
    TIMSK0 |= (1 << TOIE0);  // Enable Overflow Interrupt Enable
    TCNT0 = 0;              // Initialize Counter
}

void timer1start(void) {
    TCCR1B |= (1 << CS11);// | (1 << CS10);   // Prescaler = FCPU / 64
    TIMSK1 |= (1 << TOIE1);  // Enable Overflow Interrupt Enable
    TCNT1 = 0;              // Initialize Counter
}

int main() {

    // global interupt enable
    sei();

    portConfig();

    // timer0 aprox. get data from RTC
    // timer1 for update 7 segment
    //timer0start();
    //    UART_init();

    // aprox. 4x per second
    //timer1start();

    //UART_puts(" offhour:");
    //UART_puti(off[0]);
    //UART_puts(" offhour:");
    //UART_puti(off[1]);
    //UART_puts("\n");

    //PORTB |= (1 << PB3);


    PORTB = 0;
    PORTD = 0;

    uint8_t i, j;
    while(1) {
        PORTD ^= (1 << PD0);
        _delay_ms(10);
    }
    return 0;
}

/**
 * Timer Overflow for update time from RTC
 * and for switch
 */
ISR(TIMER1_OVF_vect) {
    //    PORTD ^= (1 << PD1);
}

/**
 * Timer Overflow for update display
 */
ISR(TIMER0_OVF_vect) {

}
