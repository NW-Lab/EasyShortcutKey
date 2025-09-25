/*
 * ESP32-S3 USB HID Keyboard - Minimal Test
 * M5Stack AtomS3での最小構成テスト
 */

#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard keyboard;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== ESP32-S3 USB HID Minimal Test ===");
  
  // USB初期化
  USB.begin();
  delay(3000);
  
  // キーボードHID初期化
  keyboard.begin();
  delay(2000);
  
  Serial.println("USB HID initialized successfully!");
  Serial.println("Waiting 5 seconds before sending test...");
  
  delay(5000);
  
  // シンプルなテストキー送信
  Serial.println("Sending 'Test' + Enter");
  keyboard.print("Test");
  keyboard.write(KEY_RETURN);
  
  Serial.println("Initial test completed!");
}

void loop() {
  static unsigned long lastTest = 0;
  
  // 10秒おきにテストメッセージ
  if (millis() - lastTest > 10000) {
    Serial.println("Auto test: sending 'Hello' + Enter");
    keyboard.print("Hello");
    keyboard.write(KEY_RETURN);
    lastTest = millis();
  }
  
  delay(100);
}