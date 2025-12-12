/*
    A light saber that include a persistance of vision mode that can display images.

    Single press button to turn on/off.
    Long press button to toggle Normal/POV mode.

    When in normal mode, the saber lights up and animates all LEDs (on/off/movement/impact).
    A double press of the button will change the color.

    When in POV mode, the saber uses the MPU6050 gyro to detect swings and lights up the LEDs accordingly.
    A double press of the button will switch the image used in POV mode.
*/

#include <Arduino.h>
#include "globals.h"
#include "debug.h"
#include <MPU6050_DMP.h>
#include "Button2.h"
#include "led.h"

static Button2 btnPower(PIN_BUTTON_PUSH);

// ----- Main code
void setup()
{
#ifdef DEBUG
    Serial.begin(921600);
    while (!Serial)
        ; // wait for serial port to connect. Needed for native USB port only
#endif // DEBUG

    // Gyro setup
    MPU6050_setup();

    // Button setup
    btnPower.setLongClickTime(1000); // 1 second for long press
    btnPower.begin(PIN_BUTTON_PUSH, INPUT_PULLUP, true);

    // initialize our LED strip and library
    LED_setup();
    LED_set(LED_BATTERY, CRGB::Green);

    DB_PRINTLN("May the force be with you!");
}

void loop()
{
    // check the button state
    btnPower.loop();
    if (btnPower.wasPressed())
    {
        switch (btnPower.read())
        {
        case single_click:
            DB_PRINTLN("single");
            break;
        case double_click:
            DB_PRINTLN("double");
            break;
        case triple_click:
            DB_PRINTLN("triple");
            break;
        case long_click:
            DB_PRINTLN("looong");
            break;
        case empty:
        default:
            break;
        }
    }

    // show any updated LEDs
    LED_loop();
}
