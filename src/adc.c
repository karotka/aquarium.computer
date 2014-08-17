#include "adc.h"
#include <stdlib.h>

/**
 */
void ADC_init(void) {
    ADMUX |= (1 << REFS0);                                // reference AVCC (5v)
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // clockiv 128
    ADCSRA |= (1 << ADEN);                                // enable ADC 8MHz/64 = 125kHz
}

void ADC_down(void) {
    ADCSRA &= ~(1 << ADEN);
}

inline void ADC_channel(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
}

inline uint16_t ADC_get(void) {
    ADCSRA |= (1 << ADSC);                  // start conversion
    while(ADCSRA & (1 << ADSC));            // wait for single conversion to finish
    //ADCSRA |= (1 << ADIF);                // reset the flag
    return (ADC);                           // return value
}

uint16_t getTemperature() {
    float Vout = 0;          // output voltage of voltage divider
    unsigned int R1 = 4473;  // nominal thermistor resistance at nominal temperature Tn
    unsigned int R2 = 6600;  // fixed resistor in voltage divider
    float Rth;               // actual thermistor resistance
    float Tn = 25 + 273.15;  // nominal temperature in degrees Celsius, convert to degrees Kelvin
    //unsigned int Bth = 3977; // device specific constant from datasheet in Kelvin
    unsigned int Bth = 5200; // device specific constant from datasheet in Kelvin
    float temp;              // temperature
    unsigned int i;

    // get the voltage divider output and convert it to volts
    for (i = 0 ; i < 8; i++) {
        Vout += (ADC_get() * ADCRESOLUTION);
    }
    Vout = Vout / 8.0;

    // calculate actual thermistor resistance
    //Rth = ((AVCC - Vout) * R2) / Vout;
    Rth = ((AVCC * R2) / Vout) - R2;

    // use thermistor equation to calculate temperature in Kelvin
    temp = (Bth * Tn) / (Bth + log(Rth / R1) * Tn);

    if (temp < 0) {
        temp = 0;
    }
    return (uint16_t)round((temp - 273.15) * 10);
}
