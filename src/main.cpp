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

#ifdef WIFI_CONNECTION
#include "wificonnection.h"
#include "webserver.h"
#endif // WIFI_CONNECTION

// -- EEPROM
Preferences preferences;
#define PREF_VERSION 1 // if setting structure has been changed, count this number up to delete all settings
#define PREF_NAMESPACE "pref"
#define PREF_KEY_VERSION "ver"

#ifdef BATTERY_VOLTAGE
#define BATTERY_VOLTAGE_USB 5       // the voltage when running via USB instead of the battery
#define BATTERY_VOLTAGE_LOW 20      // the voltage we warn the user
#define BATTERY_VOLTAGE_SHUTDOWN 18 // the voltage we shutdown to prevent damaging the battery
#define BATTERY_VOLTAGE_FULL 22.3   // the voltage of a full battery

float BatteryVoltage()
{
    // Measure battery voltage
    const float R1 = 100000.0;        // 100kΩ
    const float R2 = 10000.0;         // 10kΩ
    const float ADC_MAX = 4095.0;     // 12-bit ADC
    const float V_REF = 3.3;          // Reference voltage
    const float ALPHA = 0.05;         // low pass filter
    static float filteredBattery = 0; // use a low-pass filter to smooth battery readings

    int adcValue = analogRead(PIN_BATTERY_VOLTAGE);
    float voltage = (adcValue / ADC_MAX) * V_REF;
    float batteryVoltage = voltage * (R1 + R2) / R2;

    // take the first and filter the rest
    if (!filteredBattery)
        filteredBattery = batteryVoltage;
    else
        filteredBattery = (ALPHA * batteryVoltage) + ((1 - ALPHA) * filteredBattery);

    // return the battery voltage
    return filteredBattery;
}
#endif // BATTERY_VOLTAGE

static Button2 btnPower(PIN_BUTTON_PUSH);

// ----- Main code
void setup()
{
#ifdef DEBUG
    Serial.begin(921600);
    while (!Serial)
        ; // wait for serial port to connect. Needed for native USB port only
    DB_PRINTLN("\nStarting Lightsaber on " + String(ARDUINO_BOARD));

    // debug info about the ESP32 we are running on
    DB_PRINTLN("ESP32 Chip Model: " + String(ESP.getChipModel()));
    DB_PRINTLN("ESP32 Chip Revision: " + String(ESP.getChipRevision()));
    DB_PRINTLN("ESP32 Chip Cores: " + String(ESP.getChipCores()));
    DB_PRINTLN("ESP32 CPU Frequency: " + String(ESP.getCpuFreqMHz()) + " MHz");
    DB_PRINTLN("ESP32 Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB");
    DB_PRINTLN("ESP32 Flash Speed: " + String(ESP.getFlashChipSpeed() / 1000000) + " MHz");
    DB_PRINTLN("ESP32 PSRAM Size: " + String(ESP.getPsramSize()));
    DB_PRINTLN("ESP32 Free PSRAM: " + String(ESP.getFreePsram()));
#endif // DEBUG

    // Init preferences EEPROM, if not done before
    preferences.begin(PREF_NAMESPACE, false); // false = RW-mode
    if (preferences.getUInt(PREF_KEY_VERSION, 0) != PREF_VERSION)
    {
        preferences.clear(); // Remove all preferences under the opened namespace
        preferences.putUInt(PREF_KEY_VERSION, PREF_VERSION);
        DB_PRINTF("EEPROM init complete, all preferences deleted, new pref_version: %d\n", PREF_VERSION);
    }

    // Gyro setup
    MPU6050_setup();

    // Button setup
    btnPower.setLongClickTime(1000); // 1 second for long press
    btnPower.begin(PIN_BUTTON_PUSH, INPUT_PULLUP, true);

    // initialize our LED strip and library
    LED_setup();
    LED_set(LED_BATTERY, CRGB::Green);

#ifdef WIFI_CONNECTION
    // Connect to WiFi and setup for OTA updates
    WiFi_setup();

    // setup the AsyncWebServer and WebSocketsServer
    WebServer_setup();
#endif // WIFI_CONNECTION

    DB_PRINTLN("May the force be with you!");
}

void loop()
{
// check the battery voltage and if necessary, inform the user
#ifdef BATTERY_VOLTAGE
    EVERY_N_MILLISECONDS(5000)
    {
        static float batteryVoltage = BATTERY_VOLTAGE_FULL;
        batteryVoltage = BatteryVoltage();

        // check to see if we're running via USB instead of the battery
        if (batteryVoltage < BATTERY_VOLTAGE_USB)
            batteryVoltage = BATTERY_VOLTAGE_FULL;

        if (batteryVoltage < BATTERY_VOLTAGE_LOW)
        {
            LED_set(LED_BATTERY, CRGB::Yellow);
        }
        if (batteryVoltage <= BATTERY_VOLTAGE_SHUTDOWN)
        {
            DB_PRINTF("Battery voltage is critically low (%.1f). Entering deep sleep mode...\n", batteryVoltage);
            LED_set(LED_BATTERY, CRGB::Red);
            LED_loop();
            esp_deep_sleep_start(); // Enter deep sleep mode
            return;
        }
    }
#endif // BATTERY_VOLTAGE

#ifdef WIFI_CONNECTION
    WiFi_loop();
    WebServer_loop();
#endif // WIFI_CONNECTION

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
        }
    }

    // show any updated LEDs
    LED_loop();
}
