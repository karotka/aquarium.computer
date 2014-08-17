#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#include "adc.h"


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

    UART_init();

    uint16_t val;

    //initialize ADC
    ADC_init();
    ADC_channel(2);

    char str[20];

    while(1) {

        val = getTemperature();

        sprintf (str, "value: %d.", val);

        //UART_puts("value:");
        UART_puts(str);
        UART_puts("\n");

        //approximate 1s
        _delay_ms(500);
    }

    return 0;
}

/**
 * Timer Overflow for update time from RTC
 * and for switch
 */
ISR(TIMER1_OVF_vect) {
}

/**
 * Timer Overflow for update display
 */
ISR(TIMER0_OVF_vect) {
}
