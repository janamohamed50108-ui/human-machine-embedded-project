// =================================================
// potentiometer.cpp - pot on A1 used as a simulated temperature
// =================================================
#include "potentiometer.h"
#include "adc.h"

// Map 0..1023 to 0..1000 tenths of a degree (rounded)
unsigned int pot_read_temp_x10(void)
{
    unsigned long raw = ADC_read(ADC_CH_POT);

    return (unsigned int)((raw * POT_MAX_TEMP_X10 + ADC_MAX / 2) / ADC_MAX);
}
