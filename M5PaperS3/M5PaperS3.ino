/*
 * EasyShortcutKey M5PaperS3
 * 
 * e-inkディスプレイ搭載の超低消費電力ショートカットキーデバイス
 * タッチパネル操作でPCにショートカットキーを送信
 * BLE機能なし - 省電力優先設計
 * 
 * Hardware: M5PaperS3 (ESP32-S3 + 4.7" e-ink display + touch panel)
 * Libraries: M5EPD, USB HID, ArduinoJson, SD
 */

#include "Config.h"
#include "DisplayHandler.h"
#include "PowerManager.h"
#include "DataManager.h"
#include "KeyboardHandler.h"

// グローバルオブジェクト
DisplayHandler display;
PowerManager powerManager;
DataManager dataManager;
KeyboardHandler keyboardHandler;

// システム状態
DeviceStatus currentStatus = STATUS_STARTING;
SystemConfig systemConfig;
unsigned long lastUpdateTime = 0;
unsigned long lastTouchTime = 0;
bool systemInitialized = false;
KeyboardMode keyboardMode = MODE_USB_HID;
bool powerButtonPressed = false;

// 関数プロトタイプ
void onBatteryUpdate(BatteryInfo battery);
void onLowBattery();
void onSleep();
void onWakeup();
void handleTouchEvent();
void handleButtonPress(Button* button);
void updateSystemStatus();
void printSystemInfo();
void enterSleepMode();
void handlePowerButton();
void handleKeyboardModeSwitch();
void displayKeyboardModeStatus();

void setup() {
  // シリアル通信初期化
  Serial.begin(SERIAL_BAUD_RATE);
  delay(2000);
  
  Serial.println("=========================================");
  Serial.println("EasyShortcutKey M5PaperS3 Edition v1.0");
  Serial.println("=========================================");
  
  // データマネージャ初期化（最初に実行）
  Serial.println("[System] Initializing Data Manager...");
  dataManager.begin();
  systemConfig = dataManager.getConfig();
  
  // ディスプレイ初期化
  Serial.println("[System] Initializing Display...");
  display.begin();
  display.setConfig(systemConfig);
  display.setStatus(STATUS_STARTING);
  display.update();
  
  // 電力管理初期化
  Serial.println("[System] Initializing Power Manager...");
  powerManager.begin(&systemConfig);
  powerManager.setBatteryCallback(onBatteryUpdate);
  powerManager.setLowBatteryCallback(onLowBattery);
  powerManager.setSleepCallback(onSleep);
  powerManager.setWakeupCallback(onWakeup);
  
  // 電源ボタン設定
  pinMode(POWER_BUTTON_PIN, INPUT_PULLUP);
  
  // キーボードハンドラ初期化
  Serial.println("[System] Initializing Keyboard Handler...");
  keyboardHandler.begin();
  keyboardHandler.setMode(keyboardMode);  // 初期モードはUSB HID
  
  // ショートカットデータをディスプレイに設定
  std::vector<Button> shortcuts = dataManager.getShortcuts();
  display.setButtons(shortcuts);
  
  // 初期化完了
  currentStatus = STATUS_READY;
  display.setStatus(currentStatus);
  display.setBatteryInfo(powerManager.getBatteryInfo());
  display.showShortcuts();
  display.forceUpdate();
  
  systemInitialized = true;
  lastUpdateTime = millis();
  powerManager.resetActivityTimer();
  
  Serial.println("[System] System initialization completed!");
  Serial.println("Touch panel ready for shortcut input");
  printSystemInfo();
}

void loop() {
  if (!systemInitialized) return;
  
  unsigned long currentTime = millis();
  
  // 各コンポーネントの更新
  powerManager.update();
  
  // 電源ボタンチェック
  handlePowerButton();
  
  // タッチイベント処理
  handleTouchEvent();
  
  // ディスプレイ更新（500ms間隔）
  if (currentTime - lastUpdateTime >= 500) {
    display.update();
    updateSystemStatus();
    lastUpdateTime = currentTime;
  }
  
  // 短い遅延
  delay(10);
}

// バッテリー情報更新
void onBatteryUpdate(BatteryInfo battery) {
  display.setBatteryInfo(battery);
  
  if (battery.isLowBattery) {
    currentStatus = STATUS_LOW_BATTERY;
    display.setStatus(currentStatus);
    Serial.println("[System] Low battery warning: " + String(battery.percentage) + "%");
  }
}

// 低バッテリー警告
void onLowBattery() {
  Serial.println("[System] Low battery callback triggered");
  
  // 低バッテリー時の処理
  // - 自動スリープ時間を短縮
  // - ディスプレイ更新を高速モードに変更
  systemConfig.autoSleepTime = 10000;  // 10秒に短縮
  systemConfig.updateMode = UPDATE_MODE_FAST;
  
  display.setConfig(systemConfig);
}

// スリープ前の処理
void onSleep() {
  Serial.println("[System] Preparing for sleep...");
  
  currentStatus = STATUS_SLEEPING;
  display.setStatus(currentStatus);
  display.forceUpdate();
  
  // データを保存
  dataManager.saveConfig();
}

// ウェイクアップ時の処理
void onWakeup() {
  Serial.println("[System] Wakeup detected");
  
  // ウェイクアップ理由をチェック
  if (powerManager.isWakeupByTouch()) {
    Serial.println("[System] Wakeup by touch");
  } else if (powerManager.isWakeupByTimer()) {
    Serial.println("[System] Wakeup by timer");
  }
  
  // ステータス復帰
  currentStatus = STATUS_READY;
  display.setStatus(currentStatus);
  display.forceUpdate();
}

// タッチイベント処理
void handleTouchEvent() {
  TouchInfo touch = display.getTouch();
  
  if (!touch.isValid) return;
  
  lastTouchTime = touch.timestamp;
  powerManager.resetActivityTimer();
  
  // 現在の表示モードに応じて処理
  DisplayMode mode = display.getDisplayMode();
  
  switch (mode) {
    case MODE_SHORTCUTS: {
      // ショートカットボタンのタッチチェック
      Button* touchedButton = display.getTouchedButton(touch.x, touch.y);
      if (touchedButton) {
        handleButtonPress(touchedButton);
        break;
      }
      
      // フッター領域のタッチチェック
      if (touch.y > DISPLAY_HEIGHT - FOOTER_HEIGHT) {
        if (touch.x < 100 && display.getCurrentPage() > 0) {
          // Previous page
          display.prevPage();
        } else if (touch.x > DISPLAY_WIDTH - 100 && display.getCurrentPage() < display.getTotalPages() - 1) {
          // Next page
          display.nextPage();
        } else if (touch.x > DISPLAY_WIDTH/2 - 50 && touch.x < DISPLAY_WIDTH/2 + 50) {
          // Settings button
          display.setDisplayMode(MODE_SETTINGS);
        }
      }
      break;
    }
    
    case MODE_SETTINGS: {
      // Settings画面のボタン
      if (touch.y > DISPLAY_HEIGHT - 100 && 
          touch.x > DISPLAY_WIDTH/2 - 50 && touch.x < DISPLAY_WIDTH/2 + 50) {
        display.setDisplayMode(MODE_SHORTCUTS);
      } else if (touch.y > 100 && touch.y < 150) {
        // キーボードモード切り替えボタン
        handleKeyboardModeSwitch();
      }
      break;
    }
    
    case MODE_BATTERY_INFO: {
      // Battery Info画面の戻るボタン
      if (touch.y > DISPLAY_HEIGHT - 100 && 
          touch.x > DISPLAY_WIDTH/2 - 50 && touch.x < DISPLAY_WIDTH/2 + 50) {
        display.setDisplayMode(MODE_SHORTCUTS);
      }
      break;
    }
    
    case MODE_ABOUT: {
      // About画面の戻るボタン
      if (touch.y > DISPLAY_HEIGHT - 100 && 
          touch.x > DISPLAY_WIDTH/2 - 50 && touch.x < DISPLAY_WIDTH/2 + 50) {
        display.setDisplayMode(MODE_SHORTCUTS);
      }
      break;
    }
  }
}

// ボタン押下処理
void handleButtonPress(Button* button) {
  if (!button || button->keyCount == 0) return;
  
  Serial.println("[System] Button pressed: " + button->text);
  
  // ボタンの視覚的フィードバック
  button->isPressed = true;
  display.forceUpdate();
  
  // ショートカットコマンドを作成
  ShortcutCommand command;
  command.keyCount = button->keyCount;
  command.delay = KEY_SEND_DELAY;
  
  for (int i = 0; i < button->keyCount; i++) {
    command.keys[i] = button->keys[i];
  }
  
  // キーボード入力を送信
  currentStatus = STATUS_SENDING_KEYS;
  display.setStatus(currentStatus);
  
  keyboardHandler.sendShortcut(command);
  
  // フィードバック終了
  delay(100);
  button->isPressed = false;
  
  // ステータス復帰
  currentStatus = STATUS_READY;
  display.setStatus(currentStatus);
}

// システム状態の定期更新
void updateSystemStatus() {
  // USB HID接続状態チェック
  bool keyboardConnected = keyboardHandler.isConnected();
  BatteryInfo battery = powerManager.getBatteryInfo();
  
  DeviceStatus newStatus = currentStatus;
  
  // ステータス判定
  if (battery.isLowBattery) {
    newStatus = STATUS_LOW_BATTERY;
  } else if (!keyboardConnected) {
    newStatus = STATUS_ERROR;
  } else if (currentStatus != STATUS_SENDING_KEYS) {
    newStatus = STATUS_READY;
  }
  
  // ステータス変更
  if (newStatus != currentStatus && currentStatus != STATUS_SENDING_KEYS) {
    currentStatus = newStatus;
    display.setStatus(currentStatus);
  }
}

// システム情報表示
void printSystemInfo() {
  Serial.println("\n--- System Information ---");
  Serial.println("Device: EasyShortcutKey M5PaperS3 Edition");
  Serial.println("Hardware: M5PaperS3 (ESP32-S3 + e-ink display)");
  Serial.println("Display: 4.7\" e-ink 540x960");
  Serial.println("Input: Touch panel only (BLE disabled)");
  
  BatteryInfo battery = powerManager.getBatteryInfo();
  Serial.println("Battery: " + String(battery.percentage) + "% (" + String(battery.voltage) + "V)");
  
  Serial.println("Free Heap: " + String(ESP.getFreeHeap()) + " bytes");
  Serial.println("Data Source: " + dataManager.getCurrentDataSource());
  Serial.println("Shortcuts: " + String(dataManager.getShortcuts().size()));
  Serial.println("Layout: " + String(systemConfig.layoutColumns) + " columns");
  Serial.println("----------------------------\n");
  
  Serial.println("System ready!");
  Serial.println("- Touch shortcuts to send keys");
  Serial.println("- Ultra low power design (BLE disabled)");
  Serial.println("- Keyboard modes: USB HID / Bluetooth");
  Serial.println("- Current mode: " + String(keyboardMode == MODE_USB_HID ? "USB HID" : "Bluetooth"));
  Serial.println("- Auto sleep in " + String(systemConfig.autoSleepTime / 1000) + " seconds");
  Serial.println();
}

// 電源スリープモード
void enterSleepMode() {
  Serial.println("[System] Entering sleep mode...");
  
  display.showSleepScreen();
  display.forceUpdate();
  
  powerManager.enterSleep();
}

// 電源ボタン処理
void handlePowerButton() {
  static bool lastButtonState = HIGH;
  static unsigned long buttonPressTime = 0;
  
  bool buttonState = digitalRead(POWER_BUTTON_PIN);
  
  // ボタンが押された瞬間
  if (lastButtonState == HIGH && buttonState == LOW) {
    buttonPressTime = millis();
    powerButtonPressed = true;
  }
  
  // ボタンが離された瞬間
  if (lastButtonState == LOW && buttonState == HIGH && powerButtonPressed) {
    unsigned long pressDuration = millis() - buttonPressTime;
    
    if (pressDuration >= 2000) {  // 2秒長押し = 電源OFF
      Serial.println("[System] Power button long press - shutting down...");
      display.showShutdownScreen();
      display.forceUpdate();
      delay(2000);
      esp_deep_sleep_start();
    } else if (pressDuration >= 100) {  // 短押し = 画面ON/OFF
      Serial.println("[System] Power button short press - toggling display...");
      if (currentStatus == STATUS_SLEEPING) {
        powerManager.wakeup();
      } else {
        enterSleepMode();
      }
    }
    
    powerButtonPressed = false;
  }
  
  lastButtonState = buttonState;
}

// キーボードモード切り替え
void handleKeyboardModeSwitch() {
  if (keyboardMode == MODE_USB_HID) {
    keyboardMode = MODE_BLUETOOTH_HID;
    Serial.println("[System] Switching to Bluetooth mode...");
  } else {
    keyboardMode = MODE_USB_HID;
    Serial.println("[System] Switching to USB HID mode...");
  }
  
  keyboardHandler.setMode(keyboardMode);
  displayKeyboardModeStatus();
}

// キーボードモードステータス表示
void displayKeyboardModeStatus() {
  String modeText = (keyboardMode == MODE_USB_HID) ? "USB HID" : "Bluetooth";
  String statusText = keyboardHandler.getConnectionStatus();
  
  Serial.println("[Keyboard] Mode: " + modeText + " - " + statusText);
  
  // ディスプレイにモードを表示（settings画面で）
  display.update();
}