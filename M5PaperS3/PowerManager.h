#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include "Config.h"
#include <M5EPD.h>

class PowerManager {
private:
  BatteryInfo batteryInfo;
  SystemConfig* config;
  bool deepSleepEnabled;
  unsigned long lastActivityTime;
  unsigned long lastBatteryCheck;
  
  // ウェイクアップソース
  bool wakeupByTouch;
  bool wakeupByBLE;
  bool wakeupByTimer;
  
  // 電力モード
  enum PowerMode {
    MODE_ACTIVE,
    MODE_LIGHT_SLEEP,
    MODE_DEEP_SLEEP
  };
  PowerMode currentMode;
  
  // コールバック関数ポインタ
  void (*onBatteryUpdate)(BatteryInfo battery);
  void (*onLowBattery)();
  void (*onSleep)();
  void (*onWakeup)();

public:
  PowerManager();
  void begin(SystemConfig* cfg);
  void update();
  
  // バッテリー情報
  BatteryInfo getBatteryInfo();
  void updateBatteryInfo();
  bool isLowBattery();
  bool isCharging();
  
  // 電力管理
  void setDeepSleepEnabled(bool enabled);
  void enterDeepSleep();
  void enterLightSleep();
  void wakeup();
  void resetActivityTimer();
  
  // タイマー管理
  bool shouldSleep();
  unsigned long getIdleTime();
  void setSleepTime(unsigned long sleepTimeMs);
  
  // ウェイクアップソース設定
  void enableTouchWakeup(bool enabled);
  void enableBLEWakeup(bool enabled);
  void enableTimerWakeup(bool enabled, unsigned long intervalMs = 0);
  
  // コールバック設定
  void setBatteryCallback(void (*callback)(BatteryInfo));
  void setLowBatteryCallback(void (*callback)());
  void setSleepCallback(void (*callback)());
  void setWakeupCallback(void (*callback)());
  
  // ウェイクアップ理由取得
  bool isWakeupByTouch();
  bool isWakeupByBLE();
  bool isWakeupByTimer();
  
  // システム情報
  PowerMode getCurrentMode();
  String getPowerModeString();
  unsigned long getUptimeMs();
  
private:
  float readBatteryVoltage();
  int calculateBatteryPercentage(float voltage);
  void checkBatteryStatus();
  void handleLowBattery();
  void setupWakeupSources();
  void clearWakeupSources();
  void logPowerEvent(const String& event);
};

#endif // POWERMANAGER_H