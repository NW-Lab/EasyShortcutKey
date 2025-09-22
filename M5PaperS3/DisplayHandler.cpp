#include "DisplayHandler.h"

DisplayHandler::DisplayHandler() {
  currentStatus = STATUS_STARTING;
  currentMode = MODE_SHORTCUTS;
  lastUpdateTime = 0;
  needsUpdate = true;
  isInitialized = false;
  
  // デフォルト設定
  config.layoutColumns = DEFAULT_LAYOUT;
  config.updateMode = DEFAULT_UPDATE_MODE;
  config.autoSleepTime = AUTO_SLEEP_TIME;
  config.touchSensitivity = TOUCH_THRESHOLD;
  
  // ページ情報初期化
  pageInfo.currentPage = 0;
  pageInfo.totalPages = 1;
  pageInfo.buttonsPerPage = getButtonsPerPage();
  pageInfo.totalButtons = 0;
}

void DisplayHandler::begin() {
  Serial.println("[Display] Initializing M5EPD...");
  
  M5.begin();
  M5.EPD.SetRotation(DISPLAY_ROTATION);
  M5.EPD.Clear(true);
  
  initCanvas();
  
  isInitialized = true;
  needsUpdate = true;
  
  Serial.println("[Display] M5EPD initialized");
}

void DisplayHandler::setButtons(const std::vector<Button>& buttonList) {
  buttons = buttonList;
  pageInfo.totalButtons = buttons.size();
  pageInfo.buttonsPerPage = getButtonsPerPage();
  pageInfo.totalPages = (pageInfo.totalButtons + pageInfo.buttonsPerPage - 1) / pageInfo.buttonsPerPage;
  
  if (pageInfo.totalPages == 0) pageInfo.totalPages = 1;
  if (pageInfo.currentPage >= pageInfo.totalPages) {
    pageInfo.currentPage = pageInfo.totalPages - 1;
  }
  
  calculateButtonLayout();
  needsUpdate = true;
  
  Serial.println("[Display] Buttons updated: " + String(buttons.size()) + " buttons");
}

void DisplayHandler::setStatus(DeviceStatus status) {
  if (currentStatus != status) {
    currentStatus = status;
    needsUpdate = true;
    Serial.println("[Display] Status updated: " + String(status));
  }
}

void DisplayHandler::setBatteryInfo(const BatteryInfo& battery) {
  batteryInfo = battery;
  needsUpdate = true;
}

void DisplayHandler::setConfig(const SystemConfig& cfg) {
  bool layoutChanged = (config.layoutColumns != cfg.layoutColumns);
  config = cfg;
  
  if (layoutChanged) {
    pageInfo.buttonsPerPage = getButtonsPerPage();
    calculateButtonLayout();
  }
  
  needsUpdate = true;
}

void DisplayHandler::showShortcuts() {
  currentMode = MODE_SHORTCUTS;
  needsUpdate = true;
}

void DisplayHandler::showSettings() {
  currentMode = MODE_SETTINGS;
  needsUpdate = true;
}

void DisplayHandler::showBatteryInfo() {
  currentMode = MODE_BATTERY_INFO;
  needsUpdate = true;
}

void DisplayHandler::showAbout() {
  currentMode = MODE_ABOUT;
  needsUpdate = true;
}

void DisplayHandler::showSleepScreen() {
  clear();
  drawSleepScreen();
  forceUpdate();
}

void DisplayHandler::showShutdownScreen() {
  clear();
  drawShutdownScreen();
  forceUpdate();
}

void DisplayHandler::update() {
  if (!isInitialized || !needsUpdate) return;
  
  canvas.fillCanvas(COLOR_WHITE);
  
  switch (currentMode) {
    case MODE_SHORTCUTS:
      drawHeader();
      drawButtons();
      drawFooter();
      break;
    case MODE_SETTINGS:
      drawSettingsScreen();
      break;
    case MODE_BATTERY_INFO:
      drawBatteryInfoScreen();
      break;
    case MODE_ABOUT:
      drawAboutScreen();
      break;
  }
  
  // 更新モードに応じてディスプレイ更新
  if (config.updateMode == UPDATE_MODE_FAST) {
    canvas.pushCanvas(0, 0, UPDATE_MODE_A2);
  } else {
    canvas.pushCanvas(0, 0, UPDATE_MODE_GC16);
  }
  
  needsUpdate = false;
  lastUpdateTime = millis();
}

void DisplayHandler::forceUpdate() {
  needsUpdate = true;
  update();
}

void DisplayHandler::clear() {
  M5.EPD.Clear(true);
}

void DisplayHandler::nextPage() {
  if (pageInfo.currentPage < pageInfo.totalPages - 1) {
    pageInfo.currentPage++;
    needsUpdate = true;
    Serial.println("[Display] Next page: " + String(pageInfo.currentPage + 1) + "/" + String(pageInfo.totalPages));
  }
}

void DisplayHandler::prevPage() {
  if (pageInfo.currentPage > 0) {
    pageInfo.currentPage--;
    needsUpdate = true;
    Serial.println("[Display] Previous page: " + String(pageInfo.currentPage + 1) + "/" + String(pageInfo.totalPages));
  }
}

void DisplayHandler::setPage(int page) {
  if (page >= 0 && page < pageInfo.totalPages) {
    pageInfo.currentPage = page;
    needsUpdate = true;
  }
}

int DisplayHandler::getCurrentPage() {
  return pageInfo.currentPage;
}

int DisplayHandler::getTotalPages() {
  return pageInfo.totalPages;
}

TouchInfo DisplayHandler::getTouch() {
  TouchInfo touch;
  touch.isValid = false;
  touch.event = TOUCH_NONE;
  
  if (M5.TP.available()) {
    if (M5.TP.isFingerUp()) {
      touch.x = M5.TP.readFingerX(0);
      touch.y = M5.TP.readFingerY(0);
      touch.timestamp = millis();
      touch.event = TOUCH_TAP;
      touch.isValid = true;
      
      Serial.println("[Display] Touch detected: (" + String(touch.x) + ", " + String(touch.y) + ")");
    }
  }
  
  lastTouch = touch;
  return touch;
}

Button* DisplayHandler::getTouchedButton(int x, int y) {
  for (auto& button : buttons) {
    if (button.isVisible && isInButton(x, y, button)) {
      return &button;
    }
  }
  return nullptr;
}

bool DisplayHandler::isInButton(int x, int y, const Button& button) {
  return (x >= button.x && x <= button.x + button.width &&
          y >= button.y && y <= button.y + button.height);
}

void DisplayHandler::setDisplayMode(DisplayMode mode) {
  currentMode = mode;
  needsUpdate = true;
}

DisplayMode DisplayHandler::getDisplayMode() {
  return currentMode;
}

void DisplayHandler::initCanvas() {
  canvas.createCanvas(DISPLAY_WIDTH, DISPLAY_HEIGHT);
  canvas.setTextSize(FONT_SIZE_MEDIUM);
  canvas.setTextColor(COLOR_BLACK);
}

void DisplayHandler::drawHeader() {
  // ヘッダー背景
  canvas.fillRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, COLOR_GRAY_LIGHT);
  canvas.drawRect(0, 0, DISPLAY_WIDTH, HEADER_HEIGHT, COLOR_BLACK);
  
  // タイトル
  canvas.setTextSize(FONT_SIZE_HEADER);
  drawCenteredText(DEVICE_NAME, 0, 20, DISPLAY_WIDTH / 2, FONT_SIZE_HEADER);
  
  // バッテリーアイコン
  int batteryX = DISPLAY_WIDTH - 120;
  int batteryY = 15;
  drawBatteryIcon(batteryX, batteryY, batteryInfo.percentage);
  canvas.setTextSize(FONT_SIZE_SMALL);
  canvas.setCursor(batteryX + 35, batteryY + 5);
  canvas.printf("%d%%", batteryInfo.percentage);
  
  // 接続状態
  String statusText = "";
  switch (currentStatus) {
    case STATUS_READY:
      statusText = "� USB Ready";
      break;
    case STATUS_SENDING_KEYS:
      statusText = "⌨️ Sending";
      break;
    case STATUS_ERROR:
      statusText = "❌ Error";
      break;
    case STATUS_LOW_BATTERY:
      statusText = "🔋 Low Battery";
      break;
    default:
      statusText = "🔄 Starting";
      break;
  }
  
  canvas.setTextSize(FONT_SIZE_SMALL);
  canvas.setCursor(10, 35);
  canvas.print(statusText);
}

void DisplayHandler::drawFooter() {
  int footerY = DISPLAY_HEIGHT - FOOTER_HEIGHT;
  
  // フッター背景
  canvas.fillRect(0, footerY, DISPLAY_WIDTH, FOOTER_HEIGHT, COLOR_GRAY_LIGHT);
  canvas.drawRect(0, footerY, DISPLAY_WIDTH, FOOTER_HEIGHT, COLOR_BLACK);
  
  // ページネーション
  if (pageInfo.totalPages > 1) {
    // Previous button
    if (pageInfo.currentPage > 0) {
      drawRectButton(10, footerY + 10, 80, 30, "◀ Prev", false);
    }
    
    // Next button
    if (pageInfo.currentPage < pageInfo.totalPages - 1) {
      drawRectButton(DISPLAY_WIDTH - 90, footerY + 10, 80, 30, "Next ▶", false);
    }
    
    // Page info
    String pageText = String(pageInfo.currentPage + 1) + "/" + String(pageInfo.totalPages);
    drawCenteredText(pageText, 0, footerY + 15, DISPLAY_WIDTH, FONT_SIZE_SMALL);
  }
  
  // Settings button
  drawRectButton(DISPLAY_WIDTH/2 - 50, footerY + 10, 100, 30, "⚙️ Settings", false);
}

void DisplayHandler::drawButtons() {
  int startIndex = pageInfo.currentPage * pageInfo.buttonsPerPage;
  int endIndex = min(startIndex + pageInfo.buttonsPerPage, (int)buttons.size());
  
  for (int i = startIndex; i < endIndex; i++) {
    if (buttons[i].isVisible) {
      drawButton(buttons[i], buttons[i].isPressed);
    }
  }
}

void DisplayHandler::drawButton(const Button& button, bool pressed) {
  // ボタン背景
  int bgColor = pressed ? COLOR_GRAY_DARK : COLOR_WHITE;
  int borderColor = COLOR_BLACK;
  
  canvas.fillRect(button.x, button.y, button.width, button.height, bgColor);
  canvas.drawRect(button.x, button.y, button.width, button.height, borderColor);
  
  // ショートカットキー表示
  String keyText = formatShortcutKeys(button);
  canvas.setTextSize(FONT_SIZE_MEDIUM);
  drawCenteredText(keyText, button.x, button.y + 10, button.width, FONT_SIZE_MEDIUM);
  
  // 説明テキスト
  if (button.description.length() > 0) {
    canvas.setTextSize(FONT_SIZE_SMALL);
    drawCenteredText(button.description, button.x, button.y + button.height - 25, button.width, FONT_SIZE_SMALL);
  }
}

void DisplayHandler::drawBatteryIcon(int x, int y, int percentage) {
  // バッテリー外枠
  canvas.drawRect(x, y, 30, 15, COLOR_BLACK);
  canvas.drawRect(x + 30, y + 4, 3, 7, COLOR_BLACK);
  
  // バッテリー残量
  int fillWidth = (percentage * 26) / 100;
  int fillColor = percentage > 20 ? COLOR_GRAY_DARK : COLOR_BLACK;
  canvas.fillRect(x + 2, y + 2, fillWidth, 11, fillColor);
}

void DisplayHandler::calculateButtonLayout() {
  if (buttons.empty()) return;
  
  int buttonWidth = (config.layoutColumns == 2) ? BUTTON_WIDTH_2COL : BUTTON_WIDTH_3COL;
  int buttonHeight = (config.layoutColumns == 2) ? BUTTON_HEIGHT_2COL : BUTTON_HEIGHT_3COL;
  
  int cols = config.layoutColumns;
  int startY = HEADER_HEIGHT + BUTTON_MARGIN;
  int availableHeight = CONTENT_HEIGHT - BUTTON_MARGIN * 2;
  int rows = availableHeight / (buttonHeight + BUTTON_MARGIN);
  
  pageInfo.buttonsPerPage = cols * rows;
  
  int totalWidth = cols * buttonWidth + (cols - 1) * BUTTON_MARGIN;
  int startX = (DISPLAY_WIDTH - totalWidth) / 2;
  
  for (size_t i = 0; i < buttons.size(); i++) {
    int page = i / pageInfo.buttonsPerPage;
    int indexInPage = i % pageInfo.buttonsPerPage;
    int row = indexInPage / cols;
    int col = indexInPage % cols;
    
    buttons[i].x = startX + col * (buttonWidth + BUTTON_MARGIN);
    buttons[i].y = startY + row * (buttonHeight + BUTTON_MARGIN);
    buttons[i].width = buttonWidth;
    buttons[i].height = buttonHeight;
    buttons[i].isVisible = (page == pageInfo.currentPage);
    buttons[i].id = i;
  }
}

String DisplayHandler::formatShortcutKeys(const Button& button) {
  if (button.keyCount == 0) return button.text;
  
  String result = "";
  for (int i = 0; i < button.keyCount; i++) {
    if (i > 0) result += " + ";
    result += button.keys[i];
  }
  return result;
}

void DisplayHandler::drawSettingsScreen() {
  canvas.setTextSize(FONT_SIZE_LARGE);
  drawCenteredText("Settings", 0, 50, DISPLAY_WIDTH, FONT_SIZE_LARGE);
  
  int y = 120;
  canvas.setTextSize(FONT_SIZE_MEDIUM);
  
  // Layout setting
  canvas.setCursor(50, y);
  canvas.print("Layout: " + String(config.layoutColumns) + " columns");
  y += 60;
  
  // Update mode
  String updateModeText = (config.updateMode == UPDATE_MODE_FAST) ? "Fast" : "Quality";
  canvas.setCursor(50, y);
  canvas.print("Update Mode: " + updateModeText);
  y += 60;
  
  // Auto sleep
  canvas.setCursor(50, y);
  canvas.print("Auto Sleep: " + String(config.autoSleepTime / 1000) + "s");
  y += 60;
  
  // Keyboard mode button
  drawRectButton(50, y, DISPLAY_WIDTH - 100, 40, "Switch Keyboard Mode", false);
  y += 60;
  
  // Back button
  drawRectButton(DISPLAY_WIDTH/2 - 50, DISPLAY_HEIGHT - 100, 100, 40, "Back", false);
}

void DisplayHandler::drawBatteryInfoScreen() {
  canvas.setTextSize(FONT_SIZE_LARGE);
  drawCenteredText("Battery Info", 0, 50, DISPLAY_WIDTH, FONT_SIZE_LARGE);
  
  int y = 150;
  canvas.setTextSize(FONT_SIZE_MEDIUM);
  
  // Battery percentage
  canvas.setCursor(50, y);
  canvas.printf("Battery: %d%%", batteryInfo.percentage);
  y += 60;
  
  // Voltage
  canvas.setCursor(50, y);
  canvas.printf("Voltage: %.2fV", batteryInfo.voltage);
  y += 60;
  
  // Charging status
  canvas.setCursor(50, y);
  canvas.print("Status: " + String(batteryInfo.isCharging ? "Charging" : "Discharging"));
  
  // Battery icon (large)
  drawBatteryIcon(DISPLAY_WIDTH/2 - 50, y + 80, batteryInfo.percentage);
  
  // Back button
  drawRectButton(DISPLAY_WIDTH/2 - 50, DISPLAY_HEIGHT - 100, 100, 40, "Back", false);
}

void DisplayHandler::drawAboutScreen() {
  canvas.setTextSize(FONT_SIZE_LARGE);
  drawCenteredText("About", 0, 50, DISPLAY_WIDTH, FONT_SIZE_LARGE);
  
  int y = 150;
  canvas.setTextSize(FONT_SIZE_MEDIUM);
  
  drawCenteredText("EasyShortcutKey", 0, y, DISPLAY_WIDTH, FONT_SIZE_MEDIUM);
  y += 50;
  drawCenteredText("M5PaperS3 Edition", 0, y, DISPLAY_WIDTH, FONT_SIZE_MEDIUM);
  y += 50;
  drawCenteredText("v1.0", 0, y, DISPLAY_WIDTH, FONT_SIZE_MEDIUM);
  
  // Back button
  drawRectButton(DISPLAY_WIDTH/2 - 50, DISPLAY_HEIGHT - 100, 100, 40, "Back", false);
}

// スリープ画面描画
void DisplayHandler::drawSleepScreen() {
  canvas.fillCanvas(COLOR_WHITE);
  canvas.setTextColor(COLOR_BLACK, COLOR_WHITE);
  
  int y = DISPLAY_HEIGHT / 2 - 50;
  
  drawCenteredText("Sleep Mode", 0, y, DISPLAY_WIDTH, FONT_SIZE_LARGE);
  y += 60;
  drawCenteredText("Touch screen or press power button", 0, y, DISPLAY_WIDTH, FONT_SIZE_SMALL);
  y += 30;
  drawCenteredText("to wake up", 0, y, DISPLAY_WIDTH, FONT_SIZE_SMALL);
}

// シャットダウン画面描画
void DisplayHandler::drawShutdownScreen() {
  canvas.fillCanvas(COLOR_BLACK);
  canvas.setTextColor(COLOR_WHITE, COLOR_BLACK);
  
  int y = DISPLAY_HEIGHT / 2 - 50;
  
  drawCenteredText("Shutting Down...", 0, y, DISPLAY_WIDTH, FONT_SIZE_LARGE);
  y += 60;
  drawCenteredText("Please wait", 0, y, DISPLAY_WIDTH, FONT_SIZE_MEDIUM);
}

void DisplayHandler::drawCenteredText(const String& text, int x, int y, int width, int fontSize) {
  canvas.setTextSize(fontSize);
  int textWidth = getTextWidth(text, fontSize);
  int centeredX = x + (width - textWidth) / 2;
  canvas.setCursor(centeredX, y);
  canvas.print(text);
}

void DisplayHandler::drawRectButton(int x, int y, int width, int height, const String& text, bool pressed) {
  int bgColor = pressed ? COLOR_GRAY_DARK : COLOR_WHITE;
  canvas.fillRect(x, y, width, height, bgColor);
  canvas.drawRect(x, y, width, height, COLOR_BLACK);
  
  canvas.setTextSize(FONT_SIZE_SMALL);
  drawCenteredText(text, x, y + height/2 - 8, width, FONT_SIZE_SMALL);
}

int DisplayHandler::getTextWidth(const String& text, int fontSize) {
  // 簡易的な文字幅計算（実際のフォントに応じて調整が必要）
  return text.length() * (fontSize * 0.6);
}

int DisplayHandler::getButtonsPerPage() {
  int buttonHeight = (config.layoutColumns == 2) ? BUTTON_HEIGHT_2COL : BUTTON_HEIGHT_3COL;
  int availableHeight = CONTENT_HEIGHT - BUTTON_MARGIN * 2;
  int rows = availableHeight / (buttonHeight + BUTTON_MARGIN);
  return config.layoutColumns * rows;
}