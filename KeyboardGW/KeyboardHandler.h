#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H

#include "Config.h"
#include "USB.h"
#include "USBHIDKeyboard.h"

class KeyboardHandler {
private:
  USBHIDKeyboard keyboard;
  bool isInitialized;
  
  // キー名前からUSB HIDコードへのマッピング
  struct KeyMapping {
    String name;
    uint8_t hidCode;
    uint8_t modifier;
  };
  
  static const KeyMapping keyMappings[];
  static const int keyMappingsCount;

public:
  KeyboardHandler();
  void begin();
  void sendShortcut(ShortcutCommand command);
  void sendKey(uint8_t key, uint8_t modifiers = 0);
  void sendKeyPress(uint8_t key, uint8_t modifiers = 0);
  void sendKeyRelease();
  bool isConnected();
  
private:
  uint8_t getKeyCode(String keyName);
  uint8_t getModifierCode(String keyName);
  void pressModifiers(uint8_t modifiers);
  void releaseModifiers(uint8_t modifiers);
  void delay_ms(int ms);
};

#endif // KEYBOARDHANDLER_H