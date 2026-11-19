// =================================================
// lcd.h - HD44780 16x2 LCD driver, 4-bit mode
// Wiring: RS = D8 (PB0), EN = D9 (PB1), D4-D7 = D4-D7 (PD4-PD7), RW = GND
// =================================================
#ifndef LCD_H
#define LCD_H

#define LCD_RS PB0
#define LCD_EN PB1

void lcd_init(void);                                      // pins + init sequence
void lcd_command(unsigned char cmd);                      // send instruction byte
void lcd_data(unsigned char data);                        // send one character
void lcd_string(const char *str);                         // send a text string
void lcd_goto(unsigned char row, unsigned char col);      // move cursor (row 0/1)
void lcd_number(unsigned int value, unsigned char width); // right-aligned number

void busy_ms(unsigned int ms);                            // busy-wait, no interrupts needed

#endif // LCD_H