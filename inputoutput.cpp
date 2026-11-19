// =================================================
// inputoutput.cpp - alarm LED, buzzer and Timer0 system tick (register level)
// =================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "inputoutput.h"

// -------------------------------------------------
// OUTPUTS
// -------------------------------------------------
void io_init(void)
{
    DDRB |= (1 << BUZZER) | (1 << LED);     // outputs (OFF after reset)
}

void led_set(unsigned char on)
{
    if (on)
        PORTB |= (1 << LED);
    else
        PORTB &= ~(1 << LED);
}

void buzzer_set(unsigned char on)
{
    if (on)
        PORTB |= (1 << BUZZER);
    else
        PORTB &= ~(1 << BUZZER);
}

// LED steady ON and buzzer beeping (500 ms on / 500 ms off) while active
void alarm_output(unsigned char active, unsigned long now)
{
    if (active)
    {
        led_set(1);
        buzzer_set(((now / BEEP_MS) % 2) == 0);
    }
    else
    {
        led_set(0);
        buzzer_set(0);
    }
}

// -------------------------------------------------
// TIMER0 - 1 ms SYSTEM TICK
// 16 MHz / 64 / 250 = 1000 Hz -> interrupt every 1 ms
// -------------------------------------------------
static volatile unsigned long millis_count = 0;

void timer0_init(void)
{
    TCCR0A = (1 << WGM01);                // CTC mode
    OCR0A  = 249;
    TCCR0B = (1 << CS01) | (1 << CS00);   // prescaler 64
    TIMSK0 = (1 << OCIE0A);               // compare match interrupt
    sei();
}

ISR(TIMER0_COMPA_vect)
{
    millis_count++;
}

// Read the 32-bit counter safely (the ISR could change it mid-read)
unsigned long get_millis(void)
{
    unsigned long value;

    cli();
    value = millis_count;
    sei();

    return value;
}
