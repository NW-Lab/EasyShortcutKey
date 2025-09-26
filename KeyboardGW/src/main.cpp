/*
 * EasyShortcutKey - KeyboardGW
 * ESP32-S3 AtomS3 + BLE + USB HID統合版
 */

#include <Arduino.h>
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "M5AtomS3.h"
#include "BLEDevice.h"
#include "BLEHandler.h"
#include "BLE2902.h"
#include <esp_gap_ble_api.h>
#include <ArduinoJson.h>

// USB HIDキーボード
USBHIDKeyboard keyboard;

// BLE設定
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

bool deviceConnected = false;

// BLEHandler をグローバル変数に
BLEHandler bleHandler;

// LED制御ヘルパー
inline void setDisplayColor(uint32_t color) {
    AtomS3.dis.drawpix(color);
    AtomS3.update();
}

// BLE handling is delegated to BLEHandler

// Shortcut handling will be done via BLEHandler callbacks

void setup() {
  Serial.begin(115200);
    delay(1000);
      
  // M5AtomS3初期化（元の方法に戻す）
    AtomS3.begin(true);
    //auto cfg = M5.config();
    //AtomS3.begin(cfg);
    AtomS3.dis.setBrightness(100);  // 輝度設定追加
    
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== EasyShortcutKey KeyboardGW Starting ===");
    
    // BLE接続状態を明示的に初期化
    deviceConnected = false;
    
    // ディスプレイ初期化（赤色 = 初期化中）
    setDisplayColor(0xff0000);  // 赤色（16進数）
    Serial.println("Display: RED (Initializing)");
    
    // USB HID初期化
    Serial.println("Initializing USB HID...");
    USB.begin();
    delay(2000);
    keyboard.begin();
    delay(1000);
    Serial.println("USB HID initialized!");
    
    // BLEHandler に初期化を委譲
    Serial.println("Initializing BLE via BLEHandler...");
    bleHandler.begin();

    // ショートカット受信時のコールバックを登録
    bleHandler.setShortcutCallback([&bleHandler](ShortcutCommand cmd){
        Serial.println("=== ショートカット受信 ===");
        Serial.println("keyCount: " + String(cmd.keyCount));
        
        // 実行中はオレンジ
        setDisplayColor(0xff8000);  // オレンジ色
        
        if (cmd.keyCount > 0) {
            Serial.println("Received keys:");
            for (int i = 0; i < cmd.keyCount; ++i) {
                Serial.println("  [" + String(i) + "] = '" + cmd.keys[i] + "'");
            }
            
            // デバッグ用：受信したデータをそのまま表示
            Serial.println("DEBUG: Raw key data received");
            keyboard.print("DEBUG_KEYS:");
            for (int i = 0; i < cmd.keyCount; ++i) {
                keyboard.print(cmd.keys[i]);
                if (i < cmd.keyCount - 1) {
                    keyboard.print("+");
                }
            }
            keyboard.print(" ");
            
            // さらに詳細なテスト：単純にCmd+Cを強制送信
            Serial.println("DEBUG: Force sending Cmd+C");
            keyboard.print("FORCE_CMD_C:");
            keyboard.press(KEY_LEFT_GUI);  // Command key
            delay(10);
            keyboard.press('c');           // C key
            delay(50);
            keyboard.release('c');         // Release C
            keyboard.release(KEY_LEFT_GUI); // Release Command
            keyboard.print(" ");
            
            Serial.println("=== ショートカット送信完了 ===");
        } else {
            Serial.println("No keys to send");
            keyboard.print("NO_KEYS_RECEIVED ");
        }
        
        delay(100);
        setDisplayColor(bleHandler.isConnected() ? 0x00ff00 : 0x0000ff);
    });

    // 接続状態変化のコールバック
    bleHandler.setConnectionCallback([](bool connected){
        if (connected) {
            setDisplayColor(0x00ff00);
            Serial.println("Display: GREEN (BLE Connected)");
        } else {
            setDisplayColor(0x0000ff);
            Serial.println("Display: BLUE (BLE Disconnected, advertising restarted)");
        }
    });
    
    // 青色点灯（待機状態）
    setDisplayColor(0x0000ff);  // 青色（16進数）
    Serial.println("Display: BLUE (BLE Advertising, waiting for connection)");
    
    // 初期テスト（5秒後）
    //delay(5000);
    //Serial.println("Initial test: sending 'KeyboardGW Ready!'");
    //keyboard.print("KeyboardGW Ready!");
    //keyboard.write(KEY_RETURN);
    
    Serial.println("=== Setup completed ===");
}

void loop() {
    M5.update();
    
    // BLE接続状態の更新処理
    bleHandler.update();
    
    // ボタンが押されたらテスト送信
    if (M5.BtnA.wasPressed()) {
        Serial.println("Button pressed - sending test");
        keyboard.print("Button Test from AtomS3 ");
        keyboard.write(KEY_RETURN);
        
        // オレンジ色に一瞬点灯（ボタンテスト）
    setDisplayColor(0xff8000);  // オレンジ色（16進数）
        delay(200);
        
        // 元の色に戻す
        if (deviceConnected) {
            setDisplayColor(0x00ff00);  // 緑色（16進数）
        } else {
            setDisplayColor(0x0000ff);  // 青色（16進数）
        }
    }
    
    delay(50);
}