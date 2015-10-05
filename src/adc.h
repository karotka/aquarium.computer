#include <avr/io.h>
#include "math.h"

#define AVCC 5
// ADC resolution in volts = AVCC / 1024
#define ADCRESOLUTION 0.0048828125

void ADC_init(void);
void ADC_down(void);
inline void ADC_channel(uint8_t channel);
inline uint16_t ADC_get(void);
unsigned int getTemperature();
