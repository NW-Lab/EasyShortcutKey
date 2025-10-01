#pragma once

#include <Arduino.h>

class USBHIDClass {
public:
  void begin();
  void writeKeys(const char** keys, size_t count);
  void writeShortcut(const char** keys, size_t count); // New: for keyboard shortcuts
};

extern USBHIDClass USBHID;
