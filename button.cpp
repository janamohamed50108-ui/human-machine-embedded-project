// =================================================
// button.cpp - debounce + tap / hold detection (state machine, non-blocking)
// =================================================
#include <avr/io.h>
#include "button.h"

void button_init(button_t *b, unsigned char pin)
{
    DDRD  &= ~(1 << pin);       // input
    PORTD |=  (1 << pin);       // internal pull-up: pressed = LOW

    b->pin   = pin;
    b->state = 0;
}

unsigned char button_update(button_t *b, unsigned long now)
{
    unsigned char pressed = !(PIND & (1 << b->pin));
    unsigned char event = EVT_NONE;

    switch (b->state)
    {
        case 0:                                     // released
            if (pressed)
            {
                b->state = 1;
                b->t_change = now;
            }
            break;

        case 1:                                     // debouncing a press
            if (!pressed)
            {
                b->state = 0;                       // just a glitch
            }
            else if (now - b->t_change >= DEBOUNCE_MS)
            {
                b->state = 2;
                b->t_press = now;
                b->was_long = 0;
            }
            break;

        case 2:                                     // pressed (short so far)
            if (!pressed)
            {
                b->state = 4;
                b->t_change = now;
            }
            else if (now - b->t_press >= LONG_PRESS_MS)
            {
                b->state = 3;
                b->was_long = 1;
                b->t_repeat = now;
                event = EVT_HOLD;
            }
            break;

        case 3:                                     // held: repeat
            if (!pressed)
            {
                b->state = 4;
                b->t_change = now;
            }
            else if (now - b->t_repeat >= REPEAT_MS)
            {
                b->t_repeat = now;
                event = EVT_HOLD;
            }
            break;

        case 4:                                     // debouncing a release
            if (pressed)
            {
                b->state = b->was_long ? 3 : 2;     // bounce: still held
            }
            else if (now - b->t_change >= DEBOUNCE_MS)
            {
                b->state = 0;
                if (!b->was_long)
                    event = EVT_SHORT;              // tap reported on release
            }
            break;

        default:
            b->state = 0;                           // recover from bad state
            break;
    }

    return event;
}
