// =================================================
// potentiometer.h - optional temperature SIMULATION input
//
// The 10k pot on the board is normally wired to LCD V0 (contrast) and needs
// no code. For testing / demo without heating the LM35, a second pot can be
// wired as: outer legs 5V and GND, middle leg to A1. Then set
// USE_POT_SIMULATION to 1 in main.ino and the pot position replaces the
// LM35 reading (fully left = 0.0 C, fully right = 100.0 C).
// =================================================
#ifndef POTENTIOMETER_H
#define POTENTIOMETER_H

#define POT_MAX_TEMP_X10  1000      // pot fully right = 100.0 C

unsigned int pot_read_temp_x10(void);     // 0..1000 (tenths of a degree)

#endif // POTENTIOMETER_H
