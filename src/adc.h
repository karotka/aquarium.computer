#include <avr/io.h>
#include "math.h"

/**
 */
void ADC_init(void);
void ADC_down(void);
inline void ADC_channel(uint8_t channel);
inline uint16_t ADC_get(void);
int getTemperature();
