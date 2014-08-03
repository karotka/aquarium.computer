#include "adc.h"

/*
 */

/**
 */
void ADC_init(void) {
    ADMUX |= (1 << REFS0);                  // reference AVCC (5v)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1);  // clockiv 64
    ADCSRA |= (1 << ADEN) | (0 << ADIE);    // enable ADC 8MHz/64 = 125kHz
}

void ADC_down(void) {
    ADCSRA &= ~(1 << ADEN);
}

inline void ADC_channel(uint8_t channel) {
    ADMUX = (ADMUX & 0xf0) | (channel & 0x0f);
}

inline uint16_t ADC_get(void) {
    ADCSRA |= (1 << ADSC);                  //start conversion
    while(ADCSRA & (1 << ADSC));            //wait for single conversion to finish
    ADCSRA |= (1 << ADIF);                  //reset the flag
    return ADC;                             //return value
}
