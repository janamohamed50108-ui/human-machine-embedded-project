// =================================================
// button.h - non-blocking push button driver
// Pins: LH button = D2 (PD2), LL button = D3 (PD3), to GND, internal pull-ups
//   tap  (< 0.8 s)  -> EVT_SHORT  (limit +1, reported on release)
//   hold (>= 0.8 s) -> EVT_HOLD   (limit -1, repeats every 0.3 s while held)
// =================================================
#ifndef BUTTON_H
#define BUTTON_H

#define BTN_HIGH PD2
#define BTN_LOW  PD3

#define DEBOUNCE_MS      30     // button must be stable this long
#define LONG_PRESS_MS    800    // hold time to start decreasing
#define REPEAT_MS        300    // decrease step while still held

// Button events
#define EVT_NONE  0
#define EVT_SHORT 1
#define EVT_HOLD  2

typedef struct
{
    unsigned char pin;
    unsigned char state;         // 0 released, 1 debounce press, 2 pressed,
                                 // 3 held (long), 4 debounce release
    unsigned char was_long;
    unsigned long t_change;
    unsigned long t_press;
    unsigned long t_repeat;
} button_t;

void button_init(button_t *b, unsigned char pin);         // input + pull-up
unsigned char button_update(button_t *b, unsigned long now); // returns EVT_*

#endif // BUTTON_H
