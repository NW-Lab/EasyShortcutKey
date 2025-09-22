#include "PowerManager.h"
#include <esp_sleep.h>
#include <esp_adc_cal.h>

PowerManager::PowerManager() {
  deepSleepEnabled = true;
  lastActivityTime = 0;
  lastBatteryCheck = 0;
  currentMode = MODE_ACTIVE;
  
  wakeupByTouch = false;
  wakeupByBLE = false;
  wakeupByTimer = false;
  
  // コールバック初期化
  onBatteryUpdate = nullptr;
  onLowBattery = nullptr;
  onSleep = nullptr;
  onWakeup = nullptr;
  
  // バッテリー情報初期化
  batteryInfo.percentage = 100;
  batteryInfo.voltage = 4.2;
  batteryInfo.isCharging = false;
  batteryInfo.isLowBattery = false;
  batteryInfo.lastUpdate = 0;
}

void PowerManager::begin(SystemConfig* cfg) {
  config = cfg;
  
  Serial.println("[Power] Initializing Power Manager...");
  
  // 初期アクティビティ時間設定
  lastActivityTime = millis();
  
  // バッテリー情報更新
  updateBatteryInfo();
  
  // ウェイクアップソース設定
  setupWakeupSources();
  
  // ウェイクアップ理由をチェック
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      wakeupByTouch = true;
      Serial.println("[Power] Wakeup by touch");
      break;
    case ESP_SLEEP_WAKEUP_TIMER:
      wakeupByTimer = true;
      Serial.println("[Power] Wakeup by timer");
      break;
    case ESP_SLEEP_WAKEUP_UNDEFINED:
    default:
      Serial.println("[Power] Normal startup");
      break;
  }
  
  if (onWakeup && (wakeupByTouch || wakeupByBLE || wakeupByTimer)) {
    onWakeup();
  }
  
  Serial.println("[Power] Power Manager initialized");
  logPowerEvent("System started, mode: " + getPowerModeString());
}

void PowerManager::update() {
  unsigned long currentTime = millis();
  
  // バッテリー情報定期更新
  if (currentTime - lastBatteryCheck >= BATTERY_CHECK_INTERVAL) {
    updateBatteryInfo();
    lastBatteryCheck = currentTime;
  }
  
  // 自動スリープチェック
  if (deepSleepEnabled && shouldSleep()) {
    if (config->deepSleepEnabled) {
      enterDeepSleep();
    } else {
      enterLightSleep();
    }
  }
}

BatteryInfo PowerManager::getBatteryInfo() {
  return batteryInfo;
}

void PowerManager::updateBatteryInfo() {
  float voltage = readBatteryVoltage();
  int percentage = calculateBatteryPercentage(voltage);
  bool charging = isCharging();
  bool lowBattery = (percentage <= LOW_BATTERY_THRESHOLD);
  
  // 前回と変化があった場合のみ更新
  if (abs(batteryInfo.percentage - percentage) > 1 || 
      batteryInfo.isCharging != charging ||
      batteryInfo.isLowBattery != lowBattery) {
    
    batteryInfo.voltage = voltage;
    batteryInfo.percentage = percentage;
    batteryInfo.isCharging = charging;
    batteryInfo.isLowBattery = lowBattery;
    batteryInfo.lastUpdate = millis();
    
    Serial.printf("[Power] Battery: %d%% (%.2fV) %s\n", 
                  percentage, voltage, charging ? "Charging" : "Discharging");
    
    // コールバック呼び出し
    if (onBatteryUpdate) {
      onBatteryUpdate(batteryInfo);
    }
    
    // 低バッテリー警告
    if (lowBattery && onLowBattery) {
      handleLowBattery();
    }
  }
}

bool PowerManager::isLowBattery() {
  return batteryInfo.isLowBattery;
}

bool PowerManager::isCharging() {
  // M5PaperS3の充電検出（実装依存）
  // GPIO pin または I2C経由でのチェックが必要
  // ここでは簡易的な電圧ベース判定
  return batteryInfo.voltage > 4.1;  // 充電中は電圧が高め
}

void PowerManager::setDeepSleepEnabled(bool enabled) {
  deepSleepEnabled = enabled;
  Serial.println("[Power] Deep sleep " + String(enabled ? "enabled" : "disabled"));
}

void PowerManager::enterDeepSleep() {
  if (!deepSleepEnabled) return;
  
  Serial.println("[Power] Entering deep sleep...");
  
  if (onSleep) {
    onSleep();
  }
  
  currentMode = MODE_DEEP_SLEEP;
  logPowerEvent("Entering deep sleep");
  
  // ウェイクアップソース設定
  setupWakeupSources();
  
  // M5EPDの省電力設定
  M5.EPD.Sleep();
  
  // ESP32のdeep sleep
  esp_deep_sleep_start();
}

void PowerManager::enterLightSleep() {
  Serial.println("[Power] Entering light sleep...");
  
  if (onSleep) {
    onSleep();
  }
  
  currentMode = MODE_LIGHT_SLEEP;
  logPowerEvent("Entering light sleep");
  
  // ライトスリープは実装省略（deep sleepを推奨）
  delay(1000);
  wakeup();
}

void PowerManager::wakeup() {
  currentMode = MODE_ACTIVE;
  lastActivityTime = millis();
  
  Serial.println("[Power] Wakeup to active mode");
  logPowerEvent("Wakeup to active mode");
  
  if (onWakeup) {
    onWakeup();
  }
}

void PowerManager::resetActivityTimer() {
  lastActivityTime = millis();
}

bool PowerManager::shouldSleep() {
  return (getIdleTime() >= config->autoSleepTime);
}

unsigned long PowerManager::getIdleTime() {
  return millis() - lastActivityTime;
}

void PowerManager::setSleepTime(unsigned long sleepTimeMs) {
  config->autoSleepTime = sleepTimeMs;
  Serial.println("[Power] Auto sleep time set to: " + String(sleepTimeMs / 1000) + "s");
}

void PowerManager::enableTouchWakeup(bool enabled) {
  if (enabled) {
    // M5PaperS3のタッチパネルをウェイクアップソースに設定
    esp_sleep_enable_touchpad_wakeup();
    Serial.println("[Power] Touch wakeup enabled");
  } else {
    // タッチウェイクアップを無効にする場合の処理
    Serial.println("[Power] Touch wakeup disabled");
  }
}

void PowerManager::enableBLEWakeup(bool enabled) {
  // BLE wakeupの実装（ESP32のBLE wake機能を使用）
  if (enabled) {
    Serial.println("[Power] BLE wakeup enabled");
    // 実装は複雑なため、ここでは概念のみ
  } else {
    Serial.println("[Power] BLE wakeup disabled");
  }
}

void PowerManager::enableTimerWakeup(bool enabled, unsigned long intervalMs) {
  if (enabled && intervalMs > 0) {
    esp_sleep_enable_timer_wakeup(intervalMs * 1000);  // マイクロ秒単位
    Serial.println("[Power] Timer wakeup enabled: " + String(intervalMs / 1000) + "s");
  } else {
    Serial.println("[Power] Timer wakeup disabled");
  }
}

void PowerManager::setBatteryCallback(void (*callback)(BatteryInfo)) {
  onBatteryUpdate = callback;
}

void PowerManager::setLowBatteryCallback(void (*callback)()) {
  onLowBattery = callback;
}

void PowerManager::setSleepCallback(void (*callback)()) {
  onSleep = callback;
}

void PowerManager::setWakeupCallback(void (*callback)()) {
  onWakeup = callback;
}

bool PowerManager::isWakeupByTouch() {
  bool result = wakeupByTouch;
  wakeupByTouch = false;  // フラグリセット
  return result;
}

bool PowerManager::isWakeupByBLE() {
  bool result = wakeupByBLE;
  wakeupByBLE = false;  // フラグリセット
  return result;
}

bool PowerManager::isWakeupByTimer() {
  bool result = wakeupByTimer;
  wakeupByTimer = false;  // フラグリセット
  return result;
}

PowerManager::PowerMode PowerManager::getCurrentMode() {
  return currentMode;
}

String PowerManager::getPowerModeString() {
  switch (currentMode) {
    case MODE_ACTIVE: return "Active";
    case MODE_LIGHT_SLEEP: return "Light Sleep";
    case MODE_DEEP_SLEEP: return "Deep Sleep";
    default: return "Unknown";
  }
}

unsigned long PowerManager::getUptimeMs() {
  return millis();
}

float PowerManager::readBatteryVoltage() {
  // M5PaperS3のバッテリー電圧読み取り
  // 実際のハードウェアに応じて実装
  
  // ADCから電圧を読み取る例（要調整）
  uint32_t voltage = M5.getBatteryVoltage();  // M5EPDのAPIを使用
  return voltage / 1000.0;  // mV to V
}

int PowerManager::calculateBatteryPercentage(float voltage) {
  // リチウムイオンバッテリーの電圧-容量特性曲線
  // 3.0V(0%) ～ 4.2V(100%) の線形近似
  
  const float minVoltage = 3.0;
  const float maxVoltage = 4.2;
  
  if (voltage <= minVoltage) return 0;
  if (voltage >= maxVoltage) return 100;
  
  float percentage = ((voltage - minVoltage) / (maxVoltage - minVoltage)) * 100.0;
  return (int)round(percentage);
}

void PowerManager::checkBatteryStatus() {
  if (batteryInfo.isLowBattery && !batteryInfo.isCharging) {
    handleLowBattery();
  }
}

void PowerManager::handleLowBattery() {
  Serial.println("[Power] Low battery detected!");
  
  if (onLowBattery) {
    onLowBattery();
  }
  
  // 低バッテリー時の処理
  // - 表示輝度を下げる
  // - 自動スリープ時間を短縮
  // - 不要な機能を無効化
  
  if (batteryInfo.percentage <= 5) {
    Serial.println("[Power] Critical battery level, forcing deep sleep");
    enterDeepSleep();
  }
}

void PowerManager::setupWakeupSources() {
  // デフォルトのウェイクアップソース設定
  enableTouchWakeup(true);
  enableTimerWakeup(true, 60000);  // 1分間隔でのタイマーウェイクアップ
  
  // BLE ウェイクアップは複雑なため、必要に応じて実装
  // enableBLEWakeup(true);
}

void PowerManager::clearWakeupSources() {
  wakeupByTouch = false;
  wakeupByBLE = false;
  wakeupByTimer = false;
}

void PowerManager::logPowerEvent(const String& event) {
  Serial.println("[Power] " + event + " (uptime: " + String(getUptimeMs() / 1000) + "s)");
}