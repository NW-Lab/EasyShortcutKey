#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LEDIndicator {
public:
  static void begin();
  static void setColor(uint32_t color); // color as 0xRRGGBB
  static void blink(uint32_t color, uint16_t ms);
  static void off();
private:
  static Adafruit_NeoPixel strip;
};
