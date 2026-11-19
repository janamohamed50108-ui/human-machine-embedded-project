// =================================================
// adc.h - 10-bit ADC driver (register level)
// Reference = AVcc (5 V), prescaler 128 -> 125 kHz ADC clock
// =================================================
#ifndef ADC_H
#define ADC_H

#define ADC_CH_TEMP   0      // A0 = LM35 output
#define ADC_CH_POT    1      // A1 = potentiometer (simulation mode only)
#define ADC_MAX       1023

void ADC_init(void);
unsigned int ADC_read(unsigned char channel);   // 0..1023, 1023 on timeout

#endif // ADC_H
