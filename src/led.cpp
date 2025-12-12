#include <Arduino.h>
#include "globals.h"
#include "led.h"
#include <FastLED.h>

#define NUM_LEDS 144
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define BRIGHTNESS 32
// With parallel updates for the LEDs so fast, we get flickering if we call
// FastLED.Show every loop. Maintain a 'dirty' bit so we know when to call Show.
boolean leds_dirty = true;
CRGB leds[NUM_LEDS];

// With parallel updates for the LEDs so fast, we get flickering if we call
// FastLED.Show every loop. Maintain a 'dirty' bit so we know when to call Show.
extern boolean leds_dirty;
extern CRGB leds[NUM_LEDS]; // array of LEDs

void LED_setup()
{
    FastLED.addLeds<LED_TYPE, PIN_LED_DATA, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);
}

void LED_loop()
{
    if (leds_dirty)
    {
        FastLED.show();
        leds_dirty = false;
    }
}

void LED_set(int light, CRGB color)
{
    leds[light] = color;
    leds_dirty = true;
}
