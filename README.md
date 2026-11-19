# Car HMI Temperature Monitor

KH5048CEM Embedded System Design & Development - **Element 2: Human Machine Interface**

A car-dashboard style HMI. It reads the cabin temperature from an **LM35** sensor through the ADC,
shows it on a **16x2 LCD**, and lets the driver set a **Low limit (LL)** and a **High limit (LH)**
with two push buttons. When the temperature goes outside the limits, or the sensor fails, the
**buzzer beeps** and the **alarm LED** lights.

* **Controller:** ATmega328P @ 16 MHz (Arduino Uno board, used as hardware only)
* **Language:** native C, direct register access
  * no Arduino functions (`digitalWrite`, `analogRead`, `millis`, `delay`, ...)
  * no `util/delay.h`
  * no LCD or other libraries

| Team member | Student ID |
|---|---|
| Jana Mohamed | 202401312 |
| Hagar Ahmed | 202400574 |
| Maya Mohamed | 202400179 |

**GitHub repository:** https://github.com/janamohamed50108-ui/human-machine-embedded-project

**Demo video:** [docs/test_prototype_video.mp4](docs/test_prototype_video.mp4) (37 s)

| Time | What the video shows |
|---|---|
| 0:00 | Power-up splash screen "Car HMI Temp / Starting..." |
| 0:05 | Steady live reading (about 23.4 °C), status `OK`, limits 15 / 35 |
| 0:08 | LL raised with taps until it passes room temperature: `LOW` alarm, LED and buzzer on |
| 0:21 | LL lowered back: alarm clears to `OK` |
| 0:24 | LH lowered by holding until below room temperature: `HIGH` alarm, LED and buzzer on |
| 0:34 | LH raised back: alarm clears to `OK` |

---

## Features

* Live temperature with 0.1 °C resolution (8-sample average to reduce noise)
* Tap a button to raise its limit by 1; hold it to lower the limit (repeats while held)
* Limits stay within 0–99 °C, and LH always stays at least 1 °C above LL
* `HIGH` / `LOW` alarm with 0.5 °C hysteresis, so the buzzer doesn't flicker at a limit
* Sensor fault detection: shows `SENSOR FAULT !!` and raises the alarm
* Non-blocking design: buttons, sensor, display and alarm all keep running together
* Optional simulation mode: a potentiometer on A1 replaces the LM35 for testing

---

## Hardware

| Component | Qty |
|---|---|
| Arduino Uno (ATmega328P) | 1 |
| LCD 16x2, HD44780 compatible | 1 |
| LM35 temperature sensor | 1 |
| Push button | 2 |
| Active buzzer 5 V | 1 |
| 10 kΩ potentiometer (LCD contrast) | 1 |
| 220 Ω resistor (LCD backlight) | 1 |
| Breadboard + jumper wires | 1 set |

The alarm LED is the Uno's on-board LED **"L"** (D13), so no external LED is needed.

### Bill of Materials

The BOMs are in the [`bom/`](bom/) folder, in CSV format:

| File | Description | Total (EGP) |
|---|---|---:|
| [BOM_Standard.csv](bom/BOM_Standard.csv) | Components bought for the prototype | 772 |
| [BOM_Optimized.csv](bom/BOM_Optimized.csv) | Cost-reduced version (20 jumper wires instead of 40) | 752 |

Totals exclude the LCD and buzzer, whose prices were not recorded. Quantities are for one prototype.

### Wiring

| Uno pin | Connects to |
|---|---|
| 5V | breadboard + rail |
| GND | breadboard − rail |
| A0 | LM35 middle leg (Vout) |
| D2 | LH button (diagonally opposite leg → GND) |
| D3 | LL button (diagonally opposite leg → GND) |
| D4 – D7 | LCD pins 11 – 14 (D4 – D7) |
| D8 | LCD pin 4 (RS) |
| D9 | LCD pin 6 (E) |
| D10 | Buzzer + (long leg); buzzer − → GND |
| D13 | on-board LED "L" (no wiring) |

**LCD:**
* Pin 1 VSS, pin 5 RW and pin 16 K → GND
* Pin 2 VDD → 5V
* Pin 3 V0 → potentiometer middle leg (outer legs to 5V and GND)
* Pin 15 A → 220 Ω → 5V
* Pins 7–10 not connected

**LM35** (flat face toward you): left leg → 5V, middle → A0, right → GND.

**Buttons:** no resistors needed, because the firmware enables the internal pull-ups.

---

## Software structure

```
human-machine-embedded-project/    (repository root)
├── README.md
├── main/                          Arduino sketch folder (open main.ino)
│   ├── main.ino                   application: limits, alarm logic, LCD screens, main loop
│   ├── lcd.h / lcd.cpp            HD44780 LCD driver (4-bit mode) + busy-wait delays
│   ├── adc.h / adc.cpp            10-bit ADC driver (AVcc reference, channel select, timeout)
│   ├── inputoutput.h / .cpp       alarm LED + buzzer outputs, Timer0 1 ms system tick
│   ├── button.h / button.cpp      debounced tap / hold button state machine
│   └── potentiometer.h / .cpp     optional simulated temperature (pot on A1)
├── test/HMI_manual_passfail.robot Robot Framework test cases
├── docs/                          design document, wiring diagram, demo video
├── bom/                           BOM_Standard.csv, BOM_Optimized.csv
└── report/                        Consolidation_Report.md
```

| Module | Registers used |
|---|---|
| lcd | `DDRB`, `DDRD`, `PORTB`, `PORTD` |
| adc | `ADMUX`, `ADCSRA`, `ADCL`, `ADCH` |
| inputoutput | `DDRB`, `PORTB`, `TCCR0A`, `TCCR0B`, `OCR0A`, `TIMSK0` + `TIMER0_COMPA` ISR |
| button | `DDRD`, `PORTD`, `PIND` |

### Main loop timing

| Task | Period |
|---|---|
| Scan buttons | 5 ms |
| Read temperature + evaluate alarm | 100 ms |
| Refresh LCD | 250 ms, or immediately after a limit change |
| Update LED / buzzer | every loop |

The time base is Timer0 in CTC mode: 16 MHz / 64 / 250 = **1 ms interrupt**.

---

## Build and upload

1. The code is in the **`main`** folder. The folder name must match `main.ino`.
2. Open `main/main.ino` in the Arduino IDE. The other files appear as tabs.
3. Select **Tools → Board → Arduino Uno** and the correct **Port**.
4. Click **Upload**.

The Arduino IDE is only used as the compiler and uploader. The sketch defines its own `main()`
and calls no Arduino functions.

---

## Operation

### Screen

```
+----------------+
|Temp: 23.4C  OK |    status: OK / HIGH / LOW, or "SENSOR FAULT !!"
|LL:15C    LH:35C|
+----------------+
```

At start-up the screen shows "Car HMI Temp / Starting..." for 1.5 s. The limits always start
at **LL = 15 °C** and **LH = 35 °C**.

### Buttons

| Action | LH button (D2) | LL button (D3) |
|---|---|---|
| Tap (< 0.8 s) | LH + 1 (on release) | LL + 1 (on release) |
| Hold (≥ 0.8 s) | LH − 1, repeating every 0.3 s | LL − 1, repeating every 0.3 s |

### Alarm

| Condition | Screen | LED | Buzzer |
|---|---|---|---|
| LL ≤ T ≤ LH | `OK` | off | off |
| T > LH | `HIGH` | on | beeping |
| T < LL | `LOW` | on | beeping |
| Sensor reading invalid | `SENSOR FAULT !!` | on | beeping |

A temperature exactly on a limit is still OK. After a HIGH alarm, it clears once
T ≤ LH − 0.5 °C. After a LOW alarm, it clears once T ≥ LL + 0.5 °C.

### Settings (`main.ino`)

| Define | Default | Meaning |
|---|---|---|
| `USE_POT_SIMULATION` | 0 | 1 = use a pot on A1 (0–100 °C) instead of the LM35 |
| `LIMIT_MIN` / `LIMIT_MAX` | 0 / 99 | allowed limit range (°C) |
| `LIMIT_GAP` | 1 | minimum LH − LL (°C) |
| `HYSTERESIS` | 5 | alarm release margin, tenths of °C (0.5 °C) |
| `SENSOR_MAX_X10` | 1500 | readings above 150.0 °C count as a sensor fault |

---

## Test cases (Robot Framework)

The test cases are in `test/HMI_manual_passfail.robot`. They are manual hardware tests: each one
describes a stimulus on the real board and the expected result, and the tester records PASS or FAIL.

| ID | Test | Stimulus | Expected result | Result |
|---|---|---|---|---|
| TC-01 | System Initialization | Power on the Arduino | LCD shows the splash screen "Car HMI Temp / Starting...", then the temperature with `LL:15C  LH:35C` | ✅ PASS |
| TC-02 | Normal Temperature Reading | Temperature 25.3 °C (within the initial 15–35 °C limits) | Line 1 shows `25.3C  OK`; LED and buzzer off | ✅ PASS |
| TC-03 | Low Temperature Alert | Temperature 10 °C (below LL = 15 °C) | Status `LOW`; LED on, buzzer beeping | ✅ PASS |
| TC-04 | High Temperature Alert | Temperature 40 °C (above LH = 35 °C) | Status `HIGH`; LED on, buzzer beeping | ✅ PASS |
| TC-05 | Increase High Limit Using D2 | Short press on the D2 (LH) button | LH increases by 1 °C on release | ✅ PASS |
| TC-06 | Increase Low Limit Using D3 | Short press on the D3 (LL) button | LL increases by 1 °C on release | ✅ PASS |
| TC-07 | Alarm Output Behavior | Temperature outside the limits | On-board LED "L" steady on, buzzer beeps 0.5 s on / 0.5 s off; both off once back in range | ✅ PASS |
| TC-08 | LCD Contrast Adjustment | Turn the 10 kΩ potentiometer | Text contrast changes; text clearly readable at the set position | ✅ PASS |

**Result: 8 tests, 8 passed, 0 failed.**

### Running the tests

```
pip install robotframework
py -m robot test/HMI_manual_passfail.robot
```

Robot Framework writes `output.xml`, `log.html` and `report.html` to the current folder.
Open `report.html` in a browser to see the results.

---

## Troubleshooting

| Symptom | Cause / fix |
|---|---|
| LCD backlight on but no text | Turn the contrast pot. Check RW (pin 5) is on GND and the data wires go to LCD pins 11–14. |
| Solid blocks instead of spaces | Contrast set too high: turn the pot back slightly. |
| Temperature jumps wildly (e.g. 0 ↔ 40 °C) | A0 is not properly connected to the LM35 middle leg. Re-seat or replace the jumper wires. |
| LM35 feels hot | It is reversed. Unplug it, then turn it so the flat face is toward you: 5V, A0, GND. |
| LL or LH changes by itself | The button's legs are always connected. Rotate it 90° or use diagonal legs. |
| Buzzer silent but "L" LED lights | Check that buzzer + goes to D10. If it only clicks, it is a passive buzzer. |

---

## Limitations

* Limits are not saved to EEPROM, so they reset to 15 / 35 after power-off.
* An LM35 in this basic circuit cannot measure below about 2 °C.
