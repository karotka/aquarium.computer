#include "adc.h"
#include "math.h"

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

float getTemperature() {

    float Vout;              // output voltage of voltage divider
    unsigned int R1 = 4700;  // nominal thermistor resistance at nominal temperature Tn
    unsigned int R2 = 4700;  // fixed resistor in voltage divider
    float Rth;               // actual thermistor resistance
    float Tn = 25 + 273.15;  // nominal temperature in degrees Celsius, convert to degrees Kelvin
    unsigned int Bth = 3977; // device specific constant from datasheet in Kelvin
    float temp;              // temperature

    // ADC resolution in volts = 3.3 / 1024
    const float ADC_resolution = 0.00322265625;

    ADC_channel(ADC_CHANNEL);

    // get the voltage divider output and convert it to volts
    Vout = ADC_get() * ADC_resolution;

    // calculate actual thermistor resistance
    Rth = ((3.3 * R2) / (3.3 - Vout)) - R2;
    // use thermistor equation to calculate temperature in Kelvin
    temp = (Bth*Tn) / (Bth + log(Rth / R1) * Tn);
    // convert temperature from Kelvin to Celsius
    temp = temp - 273.15;
    // round temperature
    temp = (float)((int)(temp * 10) * 10);

    return temp;
}
