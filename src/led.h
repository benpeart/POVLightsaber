#ifndef LED_H
#define LED_H

#include <FastLED.h>

#define LED_BATTERY 0 // index of the battery status LED

void LED_setup();
void LED_loop();
void LED_set(int light, CRGB color);

#endif // LED_H
