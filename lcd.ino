// =================================================
// lcd.cpp - HD44780 16x2 LCD driver, 4-bit mode (register level)
// =================================================
#include <avr/io.h>
#include "lcd.h"

// -------------------------------------------------
// DELAYS
// The volatile counter stops the compiler shortening the loop, so the
// delay is always at least as long as asked (usually ~1.5x longer: safe).
// -------------------------------------------------
static void delay_us(unsigned int us)
{
    volatile unsigned char i;

    while (us--)
    {
        for (i = 0; i < 2; i++)
        {
        }
    }
}

void busy_ms(unsigned int ms)
{
    volatile unsigned int i;

    while (ms--)
    {
        for (i = 0; i < 2000; i++)
        {
        }
    }
}

// -------------------------------------------------
// LOW LEVEL
// -------------------------------------------------

// Generate LCD Enable pulse
static void lcd_enable(void)
{
    PORTB |= (1 << LCD_EN);
    delay_us(1);

    PORTB &= ~(1 << LCD_EN);
    delay_us(100);             // LCD instructions need 37 us: 100 gives margin
}

// Send upper 4 bits of data to LCD pins D4-D7 (PD4-PD7)
static void lcd_send_nibble(unsigned char data)
{
    PORTD = (PORTD & 0x0F) | (data & 0xF0);
    lcd_enable();
}

// -------------------------------------------------
// PUBLIC FUNCTIONS
// -------------------------------------------------

// Send command to LCD
void lcd_command(unsigned char cmd)
{
    PORTB &= ~(1 << LCD_RS);

    lcd_send_nibble(cmd);
    lcd_send_nibble(cmd << 4);

    // Clear (0x01) and Home (0x02) need 1.52 ms, the rest are fast
    if (cmd <= 0x02)
    {
        busy_ms(3);
    }
}

// Send character to LCD
void lcd_data(unsigned char data)
{
    PORTB |= (1 << LCD_RS);

    lcd_send_nibble(data);
    lcd_send_nibble(data << 4);
}

// Initialize LCD in 4-bit mode (HD44780 datasheet sequence)
void lcd_init(void)
{
    // LCD pins as outputs: data D4-D7 = PD4-PD7, RS = PB0, EN = PB1
    DDRD |= 0xF0;
    DDRB |= (1 << LCD_RS) | (1 << LCD_EN);

    busy_ms(100);              // wait for LCD power-up

    PORTB &= ~(1 << LCD_RS);
    PORTB &= ~(1 << LCD_EN);

    lcd_send_nibble(0x30);
    busy_ms(10);

    lcd_send_nibble(0x30);
    busy_ms(5);

    lcd_send_nibble(0x30);
    busy_ms(5);

    lcd_send_nibble(0x20);     // switch to 4-bit
    busy_ms(5);

    lcd_command(0x28);         // 4-bit, 2 lines, 5x8 font
    lcd_command(0x0C);         // Display ON, cursor OFF
    lcd_command(0x06);         // Increment cursor
    lcd_command(0x01);         // Clear display
}

// Display a string on LCD
void lcd_string(const char *str)
{
    while (*str)
    {
        lcd_data(*str++);
    }
}

// Move LCD cursor
void lcd_goto(unsigned char row, unsigned char col)
{
    if (row == 0)
        lcd_command(0x80 + col);
    else
        lcd_command(0xC0 + col);
}

// Print a number right-aligned in 'width' characters (spaces in front)
void lcd_number(unsigned int value, unsigned char width)
{
    char buf[5];
    unsigned char i;

    if (width > 5)
    {
        width = 5;
    }

    for (i = 0; i < width; i++)
    {
        if (i == 0 || value != 0)
        {
            buf[width - 1 - i] = (value % 10) + '0';
            value /= 10;
        }
        else
        {
            buf[width - 1 - i] = ' ';
        }
    }

    for (i = 0; i < width; i++)
    {
        lcd_data(buf[i]);
    }
}