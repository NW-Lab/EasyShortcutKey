/*
 * EasyShortcutKey KeyboardGW
 * 
 * iPhone からBLE経由でショートカットコマンドを受信し、
 * USB HIDキーボードとしてPCに送信するデバイス
 * 
 * Hardware: M5Stack AtomS3 (ESP32-S3)
 * Libraries: ESP32 BLE Arduino, USB HID, M5AtomS3, ArduinoJson
 */

#include "Config.h"
#include "LEDIndicator.h"
#include "BLEHandler.h"
#include "KeyboardHandler.h"

// グローバルオブジェクト
LEDIndicator ledIndicator;
BLEHandler bleHandler;
KeyboardHandler keyboardHandler;

// システム状態
DeviceStatus currentStatus = STATUS_STARTING;
unsigned long lastStatusUpdate = 0;
bool systemInitialized = false;

// 関数プロトタイプ
void onShortcutReceived(ShortcutCommand command);
void onBLEConnectionChanged(bool connected);
void updateSystemStatus();
void printSystemInfo();

void setup() {
  // シリアル通信初期化
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000);
  
  Serial.println("=================================");
  Serial.println("EasyShortcutKey KeyboardGW v1.0");
  Serial.println("=================================");
  
  // LED初期化
  Serial.println("[System] Initializing LED...");
  ledIndicator.begin();
  ledIndicator.setStatus(STATUS_STARTING);
  
  delay(1000);
  
  // USB HIDキーボード初期化
  Serial.println("[System] Initializing USB HID Keyboard...");
  keyboardHandler.begin();
  
  delay(500);
  
  // BLE初期化
  Serial.println("[System] Initializing BLE...");
  bleHandler.begin();
  bleHandler.setShortcutCallback(onShortcutReceived);
  bleHandler.setConnectionCallback(onBLEConnectionChanged);
  
  // 初期化完了
  currentStatus = STATUS_BLE_ADVERTISING;
  ledIndicator.setStatus(currentStatus);
  bleHandler.sendStatus(currentStatus);
  
  systemInitialized = true;
  Serial.println("[System] System initialization completed!");
  printSystemInfo();
}

void loop() {
  if (!systemInitialized) return;
  
  // 各コンポーネントの更新
  ledIndicator.update();
  bleHandler.update();
  
  // システム状態の定期更新
  updateSystemStatus();
  
  // 短い遅延
  delay(10);
}

// ショートカットコマンド受信時のコールバック
void onShortcutReceived(ShortcutCommand command) {
  Serial.println("[System] Shortcut command received");
  
  // ステータスをキー送信中に変更
  currentStatus = STATUS_SENDING_KEYS;
  ledIndicator.setStatus(currentStatus);
  bleHandler.sendStatus(currentStatus);
  
  // キーボード入力を送信
  keyboardHandler.sendShortcut(command);
  
  // ステータスを接続中に戻す
  delay(100);
  if (bleHandler.isConnected()) {
    currentStatus = STATUS_BLE_CONNECTED;
  } else {
    currentStatus = STATUS_BLE_ADVERTISING;
  }
  
  ledIndicator.setStatus(currentStatus);
  bleHandler.sendStatus(currentStatus);
}

// BLE接続状態変更時のコールバック
void onBLEConnectionChanged(bool connected) {
  Serial.println("[System] BLE connection changed: " + String(connected ? "Connected" : "Disconnected"));
  
  if (connected) {
    currentStatus = STATUS_BLE_CONNECTED;
  } else {
    currentStatus = STATUS_BLE_ADVERTISING;
  }
  
  ledIndicator.setStatus(currentStatus);
  bleHandler.sendStatus(currentStatus);
}

// システム状態の定期更新
void updateSystemStatus() {
  unsigned long currentTime = millis();
  
  // 1秒間隔でステータスチェック
  if (currentTime - lastStatusUpdate >= 1000) {
    lastStatusUpdate = currentTime;
    
    // BLE接続状態をチェック
    bool bleConnected = bleHandler.isConnected();
    bool keyboardConnected = keyboardHandler.isConnected();
    
    DeviceStatus newStatus = currentStatus;
    
    // ステータス判定
    if (!keyboardConnected) {
      newStatus = STATUS_ERROR;
      Serial.println("[System] Warning: USB HID not connected");
    } else if (bleConnected && currentStatus != STATUS_SENDING_KEYS) {
      newStatus = STATUS_BLE_CONNECTED;
    } else if (!bleConnected && currentStatus != STATUS_SENDING_KEYS) {
      newStatus = STATUS_BLE_ADVERTISING;
    }
    
    // ステータス変更があった場合
    if (newStatus != currentStatus) {
      currentStatus = newStatus;
      ledIndicator.setStatus(currentStatus);
      bleHandler.sendStatus(currentStatus);
    }
  }
}

// システム情報表示
void printSystemInfo() {
  Serial.println("\n--- System Information ---");
  Serial.println("Device: EasyShortcutKey KeyboardGW");
  Serial.println("Hardware: M5Stack AtomS3 (ESP32-S3)");
  Serial.println("BLE Device Name: " + String(BLE_DEVICE_NAME));
  Serial.println("Service UUID: " + String(BLE_SERVICE_UUID));
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Chip Model: " + String(ESP.getChipModel()));
  Serial.println("Chip Revision: " + String(ESP.getChipRevision()));
  Serial.println("Flash Size: " + String(ESP.getFlashChipSize()) + " bytes");
  Serial.println("----------------------------\n");
  
  Serial.println("System ready for BLE connections!");
  Serial.println("Waiting for iPhone to connect...\n");
}