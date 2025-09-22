#ifndef DISPLAYHANDLER_H
#define DISPLAYHANDLER_H

#include "Config.h"
#include <M5EPD.h>
#include <vector>

class DisplayHandler {
private:
  M5EPD_Canvas canvas;
  std::vector<Button> buttons;
  PageInfo pageInfo;
  SystemConfig config;
  BatteryInfo batteryInfo;
  DeviceStatus currentStatus;
  DisplayMode currentMode;
  TouchInfo lastTouch;
  
  unsigned long lastUpdateTime;
  bool needsUpdate;
  bool isInitialized;

public:
  DisplayHandler();
  void begin();
  void setButtons(const std::vector<Button>& buttonList);
  void setStatus(DeviceStatus status);
  void setBatteryInfo(const BatteryInfo& battery);
  void setConfig(const SystemConfig& cfg);
  
  // 表示制御
  void showShortcuts();
  void showSettings();
  void showBatteryInfo();
  void showAbout();
  void showSleepScreen();
  void showShutdownScreen();
  void update();
  void forceUpdate();
  void clear();
  
  // ページ制御
  void nextPage();
  void prevPage();
  void setPage(int page);
  int getCurrentPage();
  int getTotalPages();
  
  // タッチ処理
  TouchInfo getTouch();
  Button* getTouchedButton(int x, int y);
  bool isInButton(int x, int y, const Button& button);
  
  // 表示モード
  void setDisplayMode(DisplayMode mode);
  DisplayMode getDisplayMode();
  
private:
  void initCanvas();
  void drawHeader();
  void drawFooter();
  void drawButtons();
  void drawButton(const Button& button, bool pressed = false);
  void drawBatteryIcon(int x, int y, int percentage);
  void drawWiFiIcon(int x, int y, bool connected);
  void drawStatusText(const String& text, int x, int y);
  
  void calculateButtonLayout();
  void updatePageInfo();
  String formatShortcutKeys(const Button& button);
  
  // 設定画面描画
  void drawSettingsScreen();
  void drawBatteryInfoScreen();
  void drawAboutScreen();
  void drawSleepScreen();
  void drawShutdownScreen();
  
  // ユーティリティ
  void drawCenteredText(const String& text, int x, int y, int width, int fontSize);
  void drawRectButton(int x, int y, int width, int height, const String& text, bool pressed);
  int getTextWidth(const String& text, int fontSize);
  int getButtonsPerPage();
};

#endif // DISPLAYHANDLER_H