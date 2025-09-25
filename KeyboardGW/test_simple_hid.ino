/*
 * ESP32-S3 USB HID Simple Test
 * Based on: https://lang-ship.com/blog/work/esp32-s3atoms3%e3%81%a7usb%e3%83%87%e3%83%90%e3%82%a4%e3%82%b9%e3%81%a7%e3%83%9e%e3%82%a6%e3%82%b9%e3%81%a8%e3%82%ad%e3%83%bc%e3%83%9c%e3%83%bc%e3%83%89%e5%ae%9f%e9%a8%93/
 */

#include "USB.h"
#include "USBHIDKeyboard.h"
#include "M5AtomS3.h"

USBHIDKeyboard Keyboard;

void setup() {
  // M5AtomS3初期化
  auto cfg = M5.config();
  AtomS3.begin(cfg);
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Starting USB HID Keyboard Test");
  
  // USB HID初期化
  Keyboard.begin();
  USB.begin();
  
  Serial.println("USB HID initialized. Waiting 5 seconds...");
  delay(5000);
  
  // テストメッセージ送信
  Serial.println("Sending test message...");
  Keyboard.print("Hello from AtomS3!");
  Serial.println("Test message sent!");
}

void loop() {
  M5.update();
  
  // ボタンが押されたらテストメッセージを送信
  if (M5.Btn.wasPressed()) {
    Serial.println("Button pressed - sending test key");
    Keyboard.print("Button Test ");
    delay(100);
  }
  
  // 10秒おきに自動でテスト送信
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 10000) {
    Serial.println("Auto test message");
    Keyboard.print("Auto Test ");
    lastSent = millis();
  }
  
  delay(100);
}