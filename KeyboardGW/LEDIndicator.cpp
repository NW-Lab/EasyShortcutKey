#include "LEDIndicator.h"

LEDIndicator::LEDIndicator() {
  currentStatus = STATUS_STARTING;
  lastBlinkTime = 0;
  blinkState = false;
  blinkColor = LED_OFF;
  blinkInterval = 500;
}

void LEDIndicator::begin() {
  M5.begin();
  M5.dis.begin();
  M5.dis.setBrightness(50); // デフォルト輝度
  setStatus(STATUS_STARTING);
}

void LEDIndicator::setStatus(DeviceStatus status) {
  currentStatus = status;
  
  switch (status) {
    case STATUS_STARTING:
      startBlink(LED_BLUE, 300);
      Serial.println("[LED] Status: Starting (Blue Blink)");
      break;
      
    case STATUS_BLE_ADVERTISING:
      setColor(LED_BLUE);
      stopBlink();
      Serial.println("[LED] Status: BLE Advertising (Blue)");
      break;
      
    case STATUS_BLE_CONNECTED:
      setColor(LED_GREEN);
      stopBlink();
      Serial.println("[LED] Status: BLE Connected (Green)");
      break;
      
    case STATUS_SENDING_KEYS:
      startBlink(LED_WHITE, 100);
      Serial.println("[LED] Status: Sending Keys (White Blink)");
      break;
      
    case STATUS_ERROR:
      startBlink(LED_RED, 200);
      Serial.println("[LED] Status: Error (Red Blink)");
      break;
  }
}

void LEDIndicator::update() {
  // 点滅処理
  if (blinkColor != LED_OFF) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= blinkInterval) {
      blinkState = !blinkState;
      lastBlinkTime = currentTime;
      
      if (blinkState) {
        M5.dis.fillpix(blinkColor);
      } else {
        M5.dis.fillpix(LED_OFF);
      }
    }
  }
}

void LEDIndicator::setBrightness(uint8_t brightness) {
  M5.dis.setBrightness(brightness);
}

void LEDIndicator::setColor(uint32_t color) {
  M5.dis.fillpix(color);
  stopBlink();
}

void LEDIndicator::startBlink(uint32_t color, int interval) {
  blinkColor = color;
  blinkInterval = interval;
  blinkState = false;
  lastBlinkTime = millis();
}

void LEDIndicator::stopBlink() {
  blinkColor = LED_OFF;
  blinkState = false;
}