#include "KeyboardHandler.h"

// キー名とUSB HIDコードのマッピングテーブル
const KeyboardHandler::KeyMapping KeyboardHandler::keyMappings[] = {
  // 修飾キー（大文字小文字両対応） - 順序修正済み
  {"ctrl", KEY_LEFT_CTRL, 0},
  {"control", KEY_LEFT_CTRL, 0},
  {"Ctrl", KEY_LEFT_CTRL, 0},
  {"Control", KEY_LEFT_CTRL, 0},
  
  {"shift", KEY_LEFT_SHIFT, 0},
  {"Shift", KEY_LEFT_SHIFT, 0},
  
  {"alt", KEY_LEFT_ALT, 0},
  {"option", KEY_LEFT_ALT, 0},
  {"Alt", KEY_LEFT_ALT, 0},
  {"Option", KEY_LEFT_ALT, 0},
  
  {"cmd", KEY_LEFT_GUI, 0},
  {"win", KEY_LEFT_GUI, 0},
  {"gui", KEY_LEFT_GUI, 0},
  {"meta", KEY_LEFT_GUI, 0},
  {"Cmd", KEY_LEFT_GUI, 0},
  {"Win", KEY_LEFT_GUI, 0},
  {"Command", KEY_LEFT_GUI, 0},
  {"Windows", KEY_LEFT_GUI, 0},
  
  // 文字キー（大文字小文字両対応） - HIDコード直接指定
  {"a", 0x04, 0}, {"A", 0x04, 0}, {"b", 0x05, 0}, {"B", 0x05, 0},
  {"c", 0x06, 0}, {"C", 0x06, 0}, {"d", 0x07, 0}, {"D", 0x07, 0},
  {"e", 0x08, 0}, {"E", 0x08, 0}, {"f", 0x09, 0}, {"F", 0x09, 0},
  {"g", 0x0A, 0}, {"G", 0x0A, 0}, {"h", 0x0B, 0}, {"H", 0x0B, 0},
  {"i", 0x0C, 0}, {"I", 0x0C, 0}, {"j", 0x0D, 0}, {"J", 0x0D, 0},
  {"k", 0x0E, 0}, {"K", 0x0E, 0}, {"l", 0x0F, 0}, {"L", 0x0F, 0},
  {"m", 0x10, 0}, {"M", 0x10, 0}, {"n", 0x11, 0}, {"N", 0x11, 0},
  {"o", 0x12, 0}, {"O", 0x12, 0}, {"p", 0x13, 0}, {"P", 0x13, 0},
  {"q", 0x14, 0}, {"Q", 0x14, 0}, {"r", 0x15, 0}, {"R", 0x15, 0},
  {"s", 0x16, 0}, {"S", 0x16, 0}, {"t", 0x17, 0}, {"T", 0x17, 0},
  {"u", 0x18, 0}, {"U", 0x18, 0}, {"v", 0x19, 0}, {"V", 0x19, 0},
  {"w", 0x1A, 0}, {"W", 0x1A, 0}, {"x", 0x1B, 0}, {"X", 0x1B, 0},
  {"y", 0x1C, 0}, {"Y", 0x1C, 0}, {"z", 0x1D, 0}, {"Z", 0x1D, 0},
  
  // 数字キー - HIDコード直接指定
  {"0", 0x27, 0}, {"1", 0x1E, 0}, {"2", 0x1F, 0}, {"3", 0x20, 0},
  {"4", 0x21, 0}, {"5", 0x22, 0}, {"6", 0x23, 0}, {"7", 0x24, 0},
  {"8", 0x25, 0}, {"9", 0x26, 0},
  
  // ファンクションキー（大文字小文字両対応）
  {"f1", KEY_F1, 0}, {"F1", KEY_F1, 0}, {"f2", KEY_F2, 0}, {"F2", KEY_F2, 0},
  {"f3", KEY_F3, 0}, {"F3", KEY_F3, 0}, {"f4", KEY_F4, 0}, {"F4", KEY_F4, 0},
  {"f5", KEY_F5, 0}, {"F5", KEY_F5, 0}, {"f6", KEY_F6, 0}, {"F6", KEY_F6, 0},
  {"f7", KEY_F7, 0}, {"F7", KEY_F7, 0}, {"f8", KEY_F8, 0}, {"F8", KEY_F8, 0},
  {"f9", KEY_F9, 0}, {"F9", KEY_F9, 0}, {"f10", KEY_F10, 0}, {"F10", KEY_F10, 0},
  {"f11", KEY_F11, 0}, {"F11", KEY_F11, 0}, {"f12", KEY_F12, 0}, {"F12", KEY_F12, 0},
  
  // 特殊キー（大文字小文字・各種表記対応）
  {"enter", KEY_RETURN, 0}, {"Enter", KEY_RETURN, 0},
  {"return", KEY_RETURN, 0}, {"Return", KEY_RETURN, 0},
  {"space", KEY_SPACE, 0}, {"Space", KEY_SPACE, 0},
  {"tab", KEY_TAB, 0}, {"Tab", KEY_TAB, 0},
  {"backspace", KEY_BACKSPACE, 0}, {"Backspace", KEY_BACKSPACE, 0},
  {"delete", KEY_DELETE, 0}, {"Delete", KEY_DELETE, 0},
  {"escape", KEY_ESC, 0}, {"Escape", KEY_ESC, 0},
  {"esc", KEY_ESC, 0}, {"Esc", KEY_ESC, 0},
  
  // 矢印キー（記号と英語表記両対応）
  {"up", KEY_UP_ARROW, 0}, {"Up", KEY_UP_ARROW, 0}, {"↑", KEY_UP_ARROW, 0},
  {"down", KEY_DOWN_ARROW, 0}, {"Down", KEY_DOWN_ARROW, 0}, {"↓", KEY_DOWN_ARROW, 0},
  {"left", KEY_LEFT_ARROW, 0}, {"Left", KEY_LEFT_ARROW, 0}, {"←", KEY_LEFT_ARROW, 0},
  {"right", KEY_RIGHT_ARROW, 0}, {"Right", KEY_RIGHT_ARROW, 0}, {"→", KEY_RIGHT_ARROW, 0},
  
  // その他の特殊キー
  {"home", KEY_HOME, 0}, {"Home", KEY_HOME, 0},
  {"end", KEY_END, 0}, {"End", KEY_END, 0},
  {"pageup", KEY_PAGE_UP, 0}, {"PageUp", KEY_PAGE_UP, 0},
  {"pagedown", KEY_PAGE_DOWN, 0}, {"PageDown", KEY_PAGE_DOWN, 0},
  {"insert", KEY_INSERT, 0}, {"Insert", KEY_INSERT, 0},
  
  // 記号キー（一部） - HIDコード直接指定
  {"-", 0x2D, 0},      // Minus/Hyphen
  {"=", 0x2E, 0},      // Equal sign
  {"[", 0x2F, 0},      // Left bracket
  {"]", 0x30, 0},      // Right bracket
  {"\\", 0x31, 0},     // Backslash
  {";", 0x33, 0},      // Semicolon
  {"'", 0x34, 0},      // Apostrophe/Quote
  {",", 0x36, 0},      // Comma
  {".", 0x37, 0},      // Period
  {"/", 0x38, 0},      // Slash
  {"`", 0x35, 0}       // Grave accent/Tilde
};

const int KeyboardHandler::keyMappingsCount = sizeof(keyMappings) / sizeof(KeyMapping);

KeyboardHandler::KeyboardHandler() {
  isInitialized = false;
}

void KeyboardHandler::begin() {
  Serial.println("[Keyboard] Initializing USB HID Keyboard...");
  
  keyboard.begin();
  USB.begin();
  
  isInitialized = true;
  Serial.println("[Keyboard] USB HID Keyboard initialized");
}

void KeyboardHandler::sendShortcut(ShortcutCommand command) {
  if (!isInitialized) {
    Serial.println("[Keyboard] Error: Not initialized");
    return;
  }
  
  Serial.println("[Keyboard] Sending shortcut with " + String(command.keyCount) + " keys");
  
  // 修飾キーとメインキーを分別
  uint8_t modifiers = 0;
  uint8_t keys[6] = {0}; // USB HIDは最大6キー同時押し
  int keyIndex = 0;
  
  for (int i = 0; i < command.keyCount; i++) {
    String keyName = command.keys[i];
    
    // JSON側の表記をKeyboardGW側の内部表記に変換
    String normalizedKey = normalizeKeyName(keyName);
    
    uint8_t modifier = getModifierCode(normalizedKey);
    if (modifier != 0) {
      modifiers |= modifier;
      Serial.println("[Keyboard] Modifier: " + keyName + " -> " + normalizedKey);
    } else {
      uint8_t keyCode = getKeyCode(normalizedKey);
      if (keyCode != 0 && keyIndex < 6) {
        keys[keyIndex] = keyCode;
        keyIndex++;
        Serial.println("[Keyboard] Key: " + keyName + " -> " + normalizedKey + " -> " + String(keyCode));
      }
    }
  }
  
  // キーを押す
  if (modifiers != 0) {
    keyboard.pressRaw(modifiers);
    delay_ms(10);
  }
  
  for (int i = 0; i < keyIndex; i++) {
    keyboard.press(keys[i]);
    delay_ms(10);
  }
  
  // 指定された遅延
  delay_ms(command.delay);
  
  // キーを離す
  keyboard.releaseAll();
  
  Serial.println("[Keyboard] Shortcut sent successfully");
}

void KeyboardHandler::sendKey(uint8_t key, uint8_t modifiers) {
  if (!isInitialized) return;
  
  sendKeyPress(key, modifiers);
  delay_ms(KEY_SEND_DELAY);
  sendKeyRelease();
}

void KeyboardHandler::sendKeyPress(uint8_t key, uint8_t modifiers) {
  if (!isInitialized) return;
  
  if (modifiers != 0) {
    keyboard.pressRaw(modifiers);
  }
  
  if (key != 0) {
    keyboard.press(key);
  }
}

void KeyboardHandler::sendKeyRelease() {
  if (!isInitialized) return;
  
  keyboard.releaseAll();
}

bool KeyboardHandler::isConnected() {
  // USB接続状態をチェック（実装依存）
  return isInitialized;
}

uint8_t KeyboardHandler::getKeyCode(String keyName) {
  for (int i = 0; i < keyMappingsCount; i++) {
    if (keyMappings[i].name == keyName && keyMappings[i].modifier == 0) {
      return keyMappings[i].hidCode;
    }
  }
  
  Serial.println("[Keyboard] Warning: Unknown key: " + keyName);
  return 0;
}

uint8_t KeyboardHandler::getModifierCode(String keyName) {
  for (int i = 0; i < keyMappingsCount; i++) {
    if (keyMappings[i].name == keyName && keyMappings[i].modifier != 0) {
      return keyMappings[i].modifier;
    }
  }
  
  return 0;
}

String KeyboardHandler::normalizeKeyName(String keyName) {
  // キーマッピングテーブルで大文字小文字と記号の両対応済みなので
  // 基本的にはそのまま返す
  // 必要に応じて特殊な変換のみ追加
  return keyName;
}

void KeyboardHandler::pressModifiers(uint8_t modifiers) {
  if (modifiers & KEY_CTRL) keyboard.press(KEY_LEFT_CTRL);
  if (modifiers & KEY_SHIFT) keyboard.press(KEY_LEFT_SHIFT);
  if (modifiers & KEY_ALT) keyboard.press(KEY_LEFT_ALT);
  if (modifiers & KEY_GUI) keyboard.press(KEY_LEFT_GUI);
}

void KeyboardHandler::releaseModifiers(uint8_t modifiers) {
  if (modifiers & KEY_CTRL) keyboard.release(KEY_LEFT_CTRL);
  if (modifiers & KEY_SHIFT) keyboard.release(KEY_LEFT_SHIFT);
  if (modifiers & KEY_ALT) keyboard.release(KEY_LEFT_ALT);
  if (modifiers & KEY_GUI) keyboard.release(KEY_LEFT_GUI);
}

void KeyboardHandler::delay_ms(int ms) {
  if (ms > 0) {
    delay(ms);
  }
}