// =================================================
// inputoutput.h - digital outputs (alarm LED, buzzer) and the 1 ms system tick
// =================================================
#ifndef INPUTOUTPUT_H
#define INPUTOUTPUT_H

// Alarm output pins
// Buzzer = D10 (PB2), LED = D13 (PB5, on-board LED "L")
#define BUZZER PB2
#define LED    PB5

#define BEEP_MS 500            // buzzer on/off time during alarm

void io_init(void);                                    // LED + buzzer as outputs
void led_set(unsigned char on);
void buzzer_set(unsigned char on);
void alarm_output(unsigned char active, unsigned long now); // LED steady, buzzer beeping

// Timer0 1 ms tick (replaces util/delay.h and Arduino millis())
void timer0_init(void);
unsigned long get_millis(void);

#endif // INPUTOUTPUT_H
