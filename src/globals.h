#ifndef GLOBALS_H
#define GLOBALS_H

// #define WIFI_CONNECTION
// #define ARDUINO_OTA
// #define WEBSERVER
// #define SERIALINPUT
// #define MDNS_ENABLED    // include MDNS support
// #define SPIFFSEDITOR // include the SPPIFFS editor
// #define DEBUG_SPINNERS
// #define BATTERY_VOLTAGE

#include <Arduino.h>
#include <Preferences.h>

// ESP32 Pin Assignments

#define PIN_LED_DATA 14        // pin to the data input line of the WS2815b LEDs
#define PIN_BATTERY_VOLTAGE 36 // ADC pin connected to voltage divider

#define PIN_BUTTON_PUSH 17  // Push Button
#define PIN_BUTTON_RED 05   // Red LED around button
#define PIN_BUTTON_GREEN 18 // Green LED around button
#define PIN_BUTTON_BLUE 19  // Blue LED around button

#define PIN_SOUND_LRC 26  // MAX98357A Left/Right/Clock Pin
#define PIN_SOUND_BCLK 25 // MAX98357A Bit Clock Pin
#define PIN_SOUND_DIN 33  // MAX98357A Data In Pin

#define PIN_I2C_SDA 21       // MPU SDA pin
#define PIN_I2C_SCL 22       // MPU SCL pin
#define PIN_MPU_INTERRUPT 23 // MPU interrupt pin, RISING triggers interrupt

extern Preferences preferences;

#endif // GLOBALS_H
