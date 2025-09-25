/*
 * ESP32-S3 USB HID Keyboard Test
 * Based on: https://lang-ship.com/blog/work/esp32-s3atoms3%e3%81%a7usb%e3%83%87%e3%83%90%e3%82%a4%e3%82%b9%e3%81%a7%e3%83%9e%e3%82%a6%e3%82%b9%e3%81%a8%e3%82%ad%e3%83%bc%e3%83%9c%e3%83%bc%e3%83%89%e5%ae%9f%e9%a8%93/
 */

#include "USB.h"
#include "USBHIDKeyboard.h"
#include "M5AtomS3.h"

USBHIDKeyboard keyboard;

void setup() {
  // M5AtomS3初期化
  auto cfg = M5.config();
  AtomS3.begin(cfg);
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Starting ESP32-S3 USB HID Test");
  
  // USB初期化を最初に行う
  USB.begin();
  delay(2000);
  
  // HIDキーボード初期化
  keyboard.begin();
  delay(1000);
  
  Serial.println("USB HID Keyboard initialized");
  Serial.println("Waiting 5 seconds before sending test message...");
  delay(5000);
  
  // テストメッセージ送信
  Serial.println("Sending: Hello from ESP32-S3 AtomS3!");
  keyboard.print("Hello from ESP32-S3 AtomS3!");
  keyboard.write(KEY_RETURN);
  Serial.println("Test message sent!");
}

void loop() {
  M5.update();
  
  // ボタンが押されたらテストメッセージを送信
  if (M5.Btn.wasPressed()) {
    Serial.println("Button pressed - sending: Test Key Input");
    keyboard.print("Test Key Input ");
    keyboard.write(KEY_RETURN);
    delay(200);
  }
  
  // 15秒おきに自動でテスト送信
  static unsigned long lastSent = 0;
  if (millis() - lastSent > 15000) {
    Serial.println("Auto test - sending: Auto Message from AtomS3");
    keyboard.print("Auto Message from AtomS3 ");
    keyboard.write(KEY_RETURN);
    lastSent = millis();
  }
  
  delay(50);
}