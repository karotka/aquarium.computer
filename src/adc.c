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
    return (uint16_t)(ADC);             // return value
}

unsigned int getTemperature() {
    float Rth;               // actual thermistor resistance
    float Vout = 0;          // output voltage of voltage divider
    float Tn = 25 + 273.15;  // nominal temperature in degrees Celsius, convert to degrees Kelvin
    float steinhart;
    unsigned int R2 = 4500;  // fixed resistor - lower increase temp
    unsigned int Bth = 3950; // B25/100 device specific constant - lower val. bigger range
    unsigned int i;

    for (i = 0 ; i < 10; i++) {
        Vout += (float)ADC_get() * ADCRESOLUTION;
    }
    Vout = Vout / 10.0;

    // calculate actual thermistor resistance
    Rth = ((AVCC * R2) - (Vout * R2)) / Vout;

    steinhart = Rth / 100000;     // (R/Ro)
    steinhart = log(steinhart);   // ln(R/Ro)
    steinhart /= Bth;             // 1/B * ln(R/Ro)
    steinhart += 1.0 / Tn;        // + (1/To)
    steinhart = 1.0 / steinhart;  // Invert
    steinhart -= 273.15;

    return steinhart * 10;
}
