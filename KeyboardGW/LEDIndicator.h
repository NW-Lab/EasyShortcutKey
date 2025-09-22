#ifndef LEDINDICATOR_H
#define LEDINDICATOR_H

#include "Config.h"
#include <M5AtomS3.h>

class LEDIndicator {
private:
  DeviceStatus currentStatus;
  unsigned long lastBlinkTime;
  bool blinkState;
  uint32_t blinkColor;
  int blinkInterval;

public:
  LEDIndicator();
  void begin();
  void setStatus(DeviceStatus status);
  void update();
  void setBrightness(uint8_t brightness);
  
private:
  void setColor(uint32_t color);
  void startBlink(uint32_t color, int interval);
  void stopBlink();
};

#endif // LEDINDICATOR_H