#include "KeyboardHandler.h"

// キー名とUSB HIDコードのマッピングテーブル
const KeyboardHandler::KeyMapping KeyboardHandler::keyMappings[] = {
  // 修飾キー
  {"ctrl", 0, KEY_LEFT_CTRL},
  {"control", 0, KEY_LEFT_CTRL},
  {"shift", 0, KEY_LEFT_SHIFT},
  {"alt", 0, KEY_LEFT_ALT},
  {"option", 0, KEY_LEFT_ALT},
  {"cmd", 0, KEY_LEFT_GUI},
  {"win", 0, KEY_LEFT_GUI},
  {"gui", 0, KEY_LEFT_GUI},
  {"meta", 0, KEY_LEFT_GUI},
  
  // 文字キー
  {"a", KEY_A, 0}, {"b", KEY_B, 0}, {"c", KEY_C, 0}, {"d", KEY_D, 0},
  {"e", KEY_E, 0}, {"f", KEY_F, 0}, {"g", KEY_G, 0}, {"h", KEY_H, 0},
  {"i", KEY_I, 0}, {"j", KEY_J, 0}, {"k", KEY_K, 0}, {"l", KEY_L, 0},
  {"m", KEY_M, 0}, {"n", KEY_N, 0}, {"o", KEY_O, 0}, {"p", KEY_P, 0},
  {"q", KEY_Q, 0}, {"r", KEY_R, 0}, {"s", KEY_S, 0}, {"t", KEY_T, 0},
  {"u", KEY_U, 0}, {"v", KEY_V, 0}, {"w", KEY_W, 0}, {"x", KEY_X, 0},
  {"y", KEY_Y, 0}, {"z", KEY_Z, 0},
  
  // 数字キー
  {"0", KEY_0, 0}, {"1", KEY_1, 0}, {"2", KEY_2, 0}, {"3", KEY_3, 0},
  {"4", KEY_4, 0}, {"5", KEY_5, 0}, {"6", KEY_6, 0}, {"7", KEY_7, 0},
  {"8", KEY_8, 0}, {"9", KEY_9, 0},
  
  // ファンクションキー
  {"f1", KEY_F1, 0}, {"f2", KEY_F2, 0}, {"f3", KEY_F3, 0}, {"f4", KEY_F4, 0},
  {"f5", KEY_F5, 0}, {"f6", KEY_F6, 0}, {"f7", KEY_F7, 0}, {"f8", KEY_F8, 0},
  {"f9", KEY_F9, 0}, {"f10", KEY_F10, 0}, {"f11", KEY_F11, 0}, {"f12", KEY_F12, 0},
  
  // 特殊キー
  {"enter", KEY_RETURN, 0},
  {"return", KEY_RETURN, 0},
  {"space", KEY_SPACE, 0},
  {"tab", KEY_TAB, 0},
  {"backspace", KEY_BACKSPACE, 0},
  {"delete", KEY_DELETE, 0},
  {"escape", KEY_ESC, 0},
  {"esc", KEY_ESC, 0},
  
  // 矢印キー
  {"up", KEY_UP_ARROW, 0},
  {"down", KEY_DOWN_ARROW, 0},
  {"left", KEY_LEFT_ARROW, 0},
  {"right", KEY_RIGHT_ARROW, 0},
  
  // その他の特殊キー
  {"home", KEY_HOME, 0},
  {"end", KEY_END, 0},
  {"pageup", KEY_PAGE_UP, 0},
  {"pagedown", KEY_PAGE_DOWN, 0},
  {"insert", KEY_INSERT, 0},
  
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
    keyName.toLowerCase();
    
    uint8_t modifier = getModifierCode(keyName);
    if (modifier != 0) {
      modifiers |= modifier;
      Serial.println("[Keyboard] Modifier: " + keyName);
    } else {
      uint8_t keyCode = getKeyCode(keyName);
      if (keyCode != 0 && keyIndex < 6) {
        keys[keyIndex] = keyCode;
        keyIndex++;
        Serial.println("[Keyboard] Key: " + keyName + " -> " + String(keyCode));
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