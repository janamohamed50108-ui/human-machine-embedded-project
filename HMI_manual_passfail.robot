*** Settings ***
Documentation     Car HMI Temperature Monitoring System - Robot Framework test cases (manual / operator-assisted).
...               Initial Low Limit (LL) = 15 C; Initial High Limit (LH) = 35 C.
...               Hardware keywords are implemented below with pop-up dialogs.
...               The operator performs each physical step, then clicks PASS or FAIL.
Library           Dialogs
Suite Setup       Connect To Arduino
Suite Teardown    Disconnect From Arduino


*** Variables ***
${LOW_LIMIT}       15
${HIGH_LIMIT}      35
${NORMAL_TEMP}     25.3
${LOW_TEMP}        10
${HIGH_TEMP}       40
${UPDATE_DELAY}    1 second
${INIT_DELAY}      2 seconds


*** Test Cases ***
TC-01 - System Initialization
    [Documentation]    Power on the Arduino and verify LCD initialization.
    [Tags]    init    display
    Power On Arduino
    Wait For System Initialization
    Verify LCD Displays Temperature And Limits

TC-02 - Normal Temperature Reading
    [Documentation]    25.3 C is within the initial range of 15 C to 35 C.
    [Tags]    temperature    display
    Set LM35 Temperature    ${NORMAL_TEMP}
    Wait For Temperature Update
    Verify LCD Temperature    ${NORMAL_TEMP}
    Verify LCD Status    OK
    Verify Alarm Is OFF

TC-03 - Low Temperature Alert
    [Documentation]    10 C is below the lower limit of 15 C.
    [Tags]    alarm    low
    Set LM35 Temperature    ${LOW_TEMP}
    Wait For Temperature Update
    Verify LCD Status    LOW
    Verify LED Is ON
    Verify Buzzer Is Active

TC-04 - High Temperature Alert
    [Documentation]    40 C is above the upper limit of 35 C.
    [Tags]    alarm    high
    Set LM35 Temperature    ${HIGH_TEMP}
    Wait For Temperature Update
    Verify LCD Status    HIGH
    Verify LED Is ON
    Verify Buzzer Is Active

TC-05 - Increase High Limit Using D2
    [Documentation]    A short press on D2 increases LH from 35 C to 36 C.
    [Tags]    buttons    limits
    Press Button    D2
    Verify High Limit    36

TC-06 - Increase Low Limit Using D3
    [Documentation]    A short press on D3 increases LL from 15 C to 16 C.
    [Tags]    buttons    limits
    Press Button    D3
    Verify Low Limit    16

TC-07 - Alarm Output Behavior
    [Documentation]    Verify LED and buzzer during an alarm and after it clears.
    [Tags]    alarm    outputs
    Set LM35 Temperature    ${HIGH_TEMP}
    Wait For Temperature Update
    Verify LED Is ON
    Verify Buzzer Beeps With Period    1 second (0.5 s on / 0.5 s off)
    Set LM35 Temperature    ${NORMAL_TEMP}
    Wait For Temperature Update
    Verify LED Is OFF
    Verify Buzzer Is OFF

TC-08 - LCD Contrast Adjustment
    [Documentation]    Verify LCD contrast adjustment using the 10 kOhm potentiometer.
    [Tags]    display    contrast
    Adjust LCD Potentiometer
    Verify LCD Characters Are Readable


*** Keywords ***
Connect To Arduino
    Pause Execution    Connect the Arduino (or open the Wokwi simulation) with LCD, LM35, LED, buzzer and buttons D2/D3.\nClick OK when ready.

Disconnect From Arduino
    Log    Test session completed.

Power On Arduino
    Pause Execution    Power on (or reset) the Arduino / restart the Wokwi simulation.\nClick OK when it has started.

Wait For System Initialization
    Sleep    ${INIT_DELAY}

Wait For Temperature Update
    Sleep    ${UPDATE_DELAY}

Set LM35 Temperature
    [Arguments]    ${temperature}
    Pause Execution    Set the LM35 temperature to ${temperature} C\n(heat/cool the sensor, or set the value in Wokwi).\nClick OK when done.

Press Button
    [Arguments]    ${button}
    Pause Execution    Press and release button ${button} ONCE.\nClick OK when done.

Adjust LCD Potentiometer
    Pause Execution    Turn the 10 kOhm LCD contrast potentiometer through its full range.\nClick OK when done.

Verify LCD Displays Temperature And Limits
    Execute Manual Step    Does the LCD show the temperature and the limits (LL = ${LOW_LIMIT}, LH = ${HIGH_LIMIT})?    LCD does not show temperature and limits.

Verify LCD Temperature
    [Arguments]    ${expected}
    Execute Manual Step    Does the LCD show a temperature of about ${expected} C?    LCD temperature is not ${expected} C.

Verify LCD Status
    [Arguments]    ${status}
    Execute Manual Step    Does the LCD status show "${status}"?    LCD status is not ${status}.

Verify Alarm Is OFF
    Execute Manual Step    Is the alarm OFF (LED off and buzzer silent)?    Alarm is not OFF.

Verify LED Is ON
    Execute Manual Step    Is the LED ON?    LED is not ON.

Verify LED Is OFF
    Execute Manual Step    Is the LED OFF?    LED is not OFF.

Verify Buzzer Is Active
    Execute Manual Step    Is the buzzer sounding?    Buzzer is not active.

Verify Buzzer Is OFF
    Execute Manual Step    Is the buzzer silent?    Buzzer is not OFF.

Verify Buzzer Beeps With Period
    [Arguments]    ${period}
    Execute Manual Step    Does the buzzer beep with a period of about ${period}?    Buzzer period is not ${period}.

Verify High Limit
    [Arguments]    ${expected}
    Execute Manual Step    Does the high limit (LH) now show ${expected}?    High limit is not ${expected}.

Verify Low Limit
    [Arguments]    ${expected}
    Execute Manual Step    Does the low limit (LL) now show ${expected}?    Low limit is not ${expected}.

Verify LCD Characters Are Readable
    Execute Manual Step    Are the LCD characters clearly readable at the best contrast setting?    LCD characters are not readable.
