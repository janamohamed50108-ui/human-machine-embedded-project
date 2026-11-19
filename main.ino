// =================================================
// CAR HMI TEMPERATURE MONITOR - ATmega328P (Uno), LM35, LCD 16x2
// Native register-level C: no Arduino functions, no util/delay.h
// =================================================
#include <avr/io.h>

#include "lcd.h"
#include "adc.h"
#include "inputoutput.h"
#include "button.h"
#include "potentiometer.h"

// ---------------- Settings ----------------
#define USE_POT_SIMULATION 0    // 1 = read pot on A1 instead of LM35

#define LIMIT_MIN        0      // lowest allowed limit (C)
#define LIMIT_MAX        99     // highest allowed limit (C), 2 digits on LCD
#define LIMIT_GAP        1      // LH must stay at least 1 C above LL
#define HYSTERESIS       5      // 0.5 C (temperatures are in tenths)
#define SENSOR_MAX_X10   1500   // LM35 max 150.0 C, above = wiring fault
#define ADC_SAMPLES      8      // readings averaged per measurement

#define BUTTON_PERIOD_MS  5     // how often buttons are scanned
#define SENSOR_PERIOD_MS  100   // how often temperature is measured
#define DISPLAY_PERIOD_MS 250   // how often LCD is refreshed

// Alarm states
#define ALARM_NONE  0
#define ALARM_HIGH  1
#define ALARM_LOW   2
#define ALARM_FAULT 3

// ---------------- Global data ----------------
unsigned char LL = 15;                  // low limit, C
unsigned char LH = 35;                  // high limit, C
unsigned int  temperature = 0;          // tenths of a degree
unsigned char alarm_state = ALARM_NONE;

button_t btn_high;                      // D2: adjusts LH
button_t btn_low;                       // D3: adjusts LL

// =================================================
// TEMPERATURE SENSOR
// LM35 = 10 mV per C, so the voltage in mV equals the temperature x 10.
// Returns 1 if the reading is valid, 0 if the sensor is faulty.
// =================================================
unsigned char read_temperature(unsigned int *temp)
{
    unsigned long sum = 0;
    unsigned long mv;
    unsigned char i;

    // Simulation mode: pot on A1 replaces the LM35
    if (USE_POT_SIMULATION)
    {
        *temp = pot_read_temp_x10();
        return 1;
    }

    // Average several readings to reduce noise
    for (i = 0; i < ADC_SAMPLES; i++)
    {
        sum += ADC_read(ADC_CH_TEMP);
    }

    // mV = adc * 5000 / 1024, using the average of the samples (rounded)
    mv = (sum * 5000UL + (512UL * ADC_SAMPLES)) / (1024UL * ADC_SAMPLES);

    if (mv > SENSOR_MAX_X10)
    {
        return 0;              // above 150 C: wire loose or shorted to 5V
    }

    *temp = (unsigned int)mv;
    return 1;
}

// =================================================
// ALARM DECISION (with hysteresis)
// =================================================
void update_alarm(unsigned char sensor_ok)
{
    unsigned int high = LH * 10;
    unsigned int low  = LL * 10;

    if (!sensor_ok)
    {
        alarm_state = ALARM_FAULT;
    }
    else if (temperature > high)
    {
        alarm_state = ALARM_HIGH;
    }
    else if (temperature < low)
    {
        alarm_state = ALARM_LOW;
    }
    else if (alarm_state == ALARM_HIGH)
    {
        // Back in range: only clear once 0.5 C below LH (stops buzzer chattering)
        if (temperature <= high - HYSTERESIS)
            alarm_state = ALARM_NONE;
    }
    else if (alarm_state == ALARM_LOW)
    {
        if (temperature >= low + HYSTERESIS)
            alarm_state = ALARM_NONE;
    }
    else
    {
        alarm_state = ALARM_NONE;
    }
}

// =================================================
// LCD SCREENS
// =================================================

// Line 1: "Temp: 25.3C  OK " / " HIGH" / " LOW" / "SENSOR FAULT !! "
void display_temperature(void)
{
    lcd_goto(0, 0);

    if (alarm_state == ALARM_FAULT)
    {
        lcd_string("SENSOR FAULT !! ");
        return;
    }

    lcd_string("Temp:");
    lcd_number(temperature / 10, 3);   // up to 3 digits (150 C)
    lcd_data('.');
    lcd_data((temperature % 10) + '0');
    lcd_data('C');

    if (alarm_state == ALARM_HIGH)
        lcd_string(" HIGH");
    else if (alarm_state == ALARM_LOW)
        lcd_string(" LOW ");
    else
        lcd_string("  OK ");
}

// Line 2: "LL:15C    LH:35C"
void display_limits(void)
{
    lcd_goto(1, 0);

    lcd_string("LL:");
    lcd_number(LL, 2);
    lcd_string("C    LH:");
    lcd_number(LH, 2);
    lcd_data('C');
}

// =================================================
// LIMIT ADJUSTMENT
// =================================================

// Change LH by +1/-1, keeping LL + gap <= LH <= LIMIT_MAX
// Returns 1 if the value changed
unsigned char change_high(signed char delta)
{
    signed int new_lh = (signed int)LH + delta;

    if (new_lh > LIMIT_MAX || new_lh < (signed int)LL + LIMIT_GAP)
        return 0;

    LH = (unsigned char)new_lh;
    return 1;
}

// Change LL by +1/-1, keeping LIMIT_MIN <= LL <= LH - gap
unsigned char change_low(signed char delta)
{
    signed int new_ll = (signed int)LL + delta;

    if (new_ll < LIMIT_MIN || new_ll > (signed int)LH - LIMIT_GAP)
        return 0;

    LL = (unsigned char)new_ll;
    return 1;
}

// Tap = +1, hold = -1. Returns 1 if a limit changed (LCD redrawn immediately)
unsigned char check_buttons(unsigned long now)
{
    unsigned char changed = 0;
    unsigned char evt;

    evt = button_update(&btn_high, now);
    if (evt == EVT_SHORT) changed |= change_high(+1);
    if (evt == EVT_HOLD)  changed |= change_high(-1);

    evt = button_update(&btn_low, now);
    if (evt == EVT_SHORT) changed |= change_low(+1);
    if (evt == EVT_HOLD)  changed |= change_low(-1);

    return changed;
}

// =================================================
// MAIN PROGRAM
// =================================================
int main(void)
{
    unsigned long now;
    unsigned long t_button = 0, t_sensor = 0, t_display = 0;
    unsigned char redraw = 1;
    unsigned char sensor_ok;

    io_init();                          // LED + buzzer outputs
    button_init(&btn_high, BTN_HIGH);   // D2 input with pull-up
    button_init(&btn_low,  BTN_LOW);    // D3 input with pull-up
    ADC_init();
    lcd_init();                         // LCD first: it does not depend on the timer
    timer0_init();                      // 1 ms tick + interrupts on

    // Splash screen
    lcd_goto(0, 0);
    lcd_string("  Car HMI Temp  ");
    lcd_goto(1, 0);
    lcd_string("  Starting...   ");
    busy_ms(1500);

    // Continuous monitoring loop - nothing here blocks,
    // so buttons, sensor, display and alarm all keep running
    while (1)
    {
        now = get_millis();

        if (now - t_button >= BUTTON_PERIOD_MS)
        {
            t_button = now;
            if (check_buttons(now))
                redraw = 1;
        }

        if (now - t_sensor >= SENSOR_PERIOD_MS)
        {
            t_sensor = now;
            sensor_ok = read_temperature(&temperature);
            update_alarm(sensor_ok);
        }

        if (redraw || (now - t_display >= DISPLAY_PERIOD_MS))
        {
            t_display = now;
            redraw = 0;
            display_temperature();
            display_limits();
        }

        alarm_output(alarm_state != ALARM_NONE, now);
    }
}