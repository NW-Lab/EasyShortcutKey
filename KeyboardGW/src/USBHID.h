#pragma once

#include <Arduino.h>

class USBHIDClass {
public:
  void begin();
  void writeKeys(const char** keys, size_t count);
};

extern USBHIDClass USBHID;
