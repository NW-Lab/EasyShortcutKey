#include "LEDIndicator.h"
#include "Config.h"

// Initialize strip: LED_COUNT pixels on LED_PIN, NEO_GRB + NEO_KHZ800
Adafruit_NeoPixel LEDIndicator::strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Default brightness (0-255). Lower value -> dimmer.
static const uint8_t DEFAULT_BRIGHTNESS = 32; // dimmer

// Keep track of current (active) color so we can restore after a blink
static uint32_t currentColor = LED_OFF;

void LEDIndicator::begin() {
  strip.begin();
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  // initialize to off
  off();
}

void LEDIndicator::setColor(uint32_t color) {
  // Adafruit expects color format as RGB tuple; we pass 0xRRGGBB
  for (int i = 0; i < strip.numPixels(); ++i) {
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;
    strip.setPixelColor(i, strip.Color(r, g, b));
  }
  strip.show();
  currentColor = color;
}

void LEDIndicator::blink(uint32_t color, uint16_t ms) {
  uint32_t prev = currentColor;
  setColor(color);
  delay(ms);
  // restore previous color (could be LED_OFF, blue, or green)
  setColor(prev);
}

void LEDIndicator::off() {
  for (int i = 0; i < strip.numPixels(); ++i) strip.setPixelColor(i, 0);
  strip.show();
  currentColor = LED_OFF;
}
