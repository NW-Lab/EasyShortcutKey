/*
 * ESP32-S3 USB HID Keyboard Test
 * 最小限のテストコード
 */
#include "USB.h"
#include "USBHIDKeyboard.h"

USBHIDKeyboard Keyboard;

void setup() {
  Serial.begin(115200);
  
  // USB HID初期化
  Keyboard.begin();
  USB.begin();
  
  Serial.println("USB HID Keyboard Test Started");
  delay(2000);
  
  // 5秒後に自動でテスト文字を送信
  Serial.println("Sending test keys in 5 seconds...");
  delay(5000);
  
  // テストキー送信
  Keyboard.print("Hello World!");
  Serial.println("Test keys sent!");
}

void loop() {
  // 10秒おきにテストキーを送信
  delay(10000);
  Keyboard.print("Test ");
  Serial.println("Test key sent!");
}