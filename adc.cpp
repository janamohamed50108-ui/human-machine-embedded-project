// =================================================
// adc.cpp - 10-bit ADC driver (register level)
// =================================================
#include <avr/io.h>
#include "adc.h"

// Initialize ADC using AVCC reference, prescaler 128 (125 kHz)
void ADC_init(void)
{
    ADMUX = (1 << REFS0);

    ADCSRA = (1 << ADEN) |
             (1 << ADPS2) |
             (1 << ADPS1) |
             (1 << ADPS0);
}

// Read one analog channel (0..7)
unsigned int ADC_read(unsigned char channel)
{
    unsigned int timeout = 10000;
    unsigned char low;

    // Select channel, keep the reference bits
    ADMUX = (ADMUX & 0xF0) | (channel & 0x07);

    // Start ADC conversion
    ADCSRA |= (1 << ADSC);

    // Wait for the conversion, but never forever
    while ((ADCSRA & (1 << ADSC)) && timeout)
    {
        timeout--;
    }

    if (timeout == 0)
    {
        return ADC_MAX;        // treated as a sensor fault
    }

    low = ADCL;                // ADCL must be read before ADCH
    return ((unsigned int)ADCH << 8) | low;
}
