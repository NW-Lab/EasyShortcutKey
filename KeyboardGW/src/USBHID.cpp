#include "USBHID.h"

USBHIDClass USBHID;

void USBHIDClass::begin() {
  // Placeholder: if USE_USB_HID is enabled, initialize TinyUSB HID here.
  Serial.println("USBHID: begin (placeholder)");
}

void USBHIDClass::writeKeys(const char** keys, size_t count) {
  // Placeholder implementation: print keys to serial for now.
  Serial.print("USBHID: writeKeys: ");
  for (size_t i = 0; i < count; ++i) {
    Serial.print(keys[i]);
    Serial.print(' ');
  }
  Serial.println();
}
