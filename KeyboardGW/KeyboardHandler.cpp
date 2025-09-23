#include "KeyboardHandler.h"

// キー名とUSB HIDコードのマッピングテーブル
const KeyboardHandler::KeyMapping KeyboardHandler::keyMappings[] = {
  // 修飾キー（大文字小文字両対応）
  {"ctrl", 0, KEY_LEFT_CTRL},
  {"control", 0, KEY_LEFT_CTRL},
  {"Ctrl", 0, KEY_LEFT_CTRL},
  {"Control", 0, KEY_LEFT_CTRL},
  
  {"shift", 0, KEY_LEFT_SHIFT},
  {"Shift", 0, KEY_LEFT_SHIFT},
  
  {"alt", 0, KEY_LEFT_ALT},
  {"option", 0, KEY_LEFT_ALT},
  {"Alt", 0, KEY_LEFT_ALT},
  {"Option", 0, KEY_LEFT_ALT},
  
  {"cmd", 0, KEY_LEFT_GUI},
  {"win", 0, KEY_LEFT_GUI},
  {"gui", 0, KEY_LEFT_GUI},
  {"meta", 0, KEY_LEFT_GUI},
  {"Cmd", 0, KEY_LEFT_GUI},
  {"Win", 0, KEY_LEFT_GUI},
  {"Command", 0, KEY_LEFT_GUI},
  {"Windows", 0, KEY_LEFT_GUI},
  
  // 文字キー（大文字小文字両対応）
  {"a", KEY_A, 0}, {"A", KEY_A, 0}, {"b", KEY_B, 0}, {"B", KEY_B, 0},
  {"c", KEY_C, 0}, {"C", KEY_C, 0}, {"d", KEY_D, 0}, {"D", KEY_D, 0},
  {"e", KEY_E, 0}, {"E", KEY_E, 0}, {"f", KEY_F, 0}, {"F", KEY_F, 0},
  {"g", KEY_G, 0}, {"G", KEY_G, 0}, {"h", KEY_H, 0}, {"H", KEY_H, 0},
  {"i", KEY_I, 0}, {"I", KEY_I, 0}, {"j", KEY_J, 0}, {"J", KEY_J, 0},
  {"k", KEY_K, 0}, {"K", KEY_K, 0}, {"l", KEY_L, 0}, {"L", KEY_L, 0},
  {"m", KEY_M, 0}, {"M", KEY_M, 0}, {"n", KEY_N, 0}, {"N", KEY_N, 0},
  {"o", KEY_O, 0}, {"O", KEY_O, 0}, {"p", KEY_P, 0}, {"P", KEY_P, 0},
  {"q", KEY_Q, 0}, {"Q", KEY_Q, 0}, {"r", KEY_R, 0}, {"R", KEY_R, 0},
  {"s", KEY_S, 0}, {"S", KEY_S, 0}, {"t", KEY_T, 0}, {"T", KEY_T, 0},
  {"u", KEY_U, 0}, {"U", KEY_U, 0}, {"v", KEY_V, 0}, {"V", KEY_V, 0},
  {"w", KEY_W, 0}, {"W", KEY_W, 0}, {"x", KEY_X, 0}, {"X", KEY_X, 0},
  {"y", KEY_Y, 0}, {"Y", KEY_Y, 0}, {"z", KEY_Z, 0}, {"Z", KEY_Z, 0},
  
  // 数字キー
  {"0", KEY_0, 0}, {"1", KEY_1, 0}, {"2", KEY_2, 0}, {"3", KEY_3, 0},
  {"4", KEY_4, 0}, {"5", KEY_5, 0}, {"6", KEY_6, 0}, {"7", KEY_7, 0},
  {"8", KEY_8, 0}, {"9", KEY_9, 0},
  
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
  
  // 記号キー（一部）
  {"-", KEY_MINUS, 0},
  {"=", KEY_EQUAL, 0},
  {"[", KEY_LEFT_BRACE, 0},
  {"]", KEY_RIGHT_BRACE, 0},
  {"\\", KEY_BACKSLASH, 0},
  {";", KEY_SEMICOLON, 0},
  {"'", KEY_QUOTE, 0},
  {",", KEY_COMMA, 0},
  {".", KEY_PERIOD, 0},
  {"/", KEY_SLASH, 0},
  {"`", KEY_TILDE, 0}
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