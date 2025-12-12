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

// Adafruit Feather RP2040 Pin Assignments

#define PIN_LED_DATA 12        // pin to the data input line of the WS2815b LEDs
#define PIN_BATTERY_VOLTAGE 29 // ADC pin connected to voltage divider

#define PIN_BUTTON_PUSH 9   // Push Button
#define PIN_BUTTON_RED 6    // Red LED around button
#define PIN_BUTTON_GREEN 7  // Green LED around button
#define PIN_BUTTON_BLUE 8   // Blue LED around button

#define PIN_SOUND_LRC 4   // MAX98357A Left/Right/Clock Pin
#define PIN_SOUND_BCLK 3  // MAX98357A Bit Clock Pin
#define PIN_SOUND_DIN 2   // MAX98357A Data In Pin

#define PIN_I2C_SDA 24       // MPU SDA pin (I2C1)
#define PIN_I2C_SCL 25       // MPU SCL pin (I2C1)
#define PIN_MPU_INTERRUPT 11 // MPU interrupt pin, RISING triggers interrupt

#endif // GLOBALS_H
