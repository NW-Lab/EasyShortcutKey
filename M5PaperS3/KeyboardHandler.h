#ifndef KEYBOARDHANDLER_H
#define KEYBOARDHANDLER_H

#include "Config.h"
#include "USB.h"
#include "USBHIDKeyboard.h"
#include "BleKeyboard.h"

class KeyboardHandler {
private:
  USBHIDKeyboard usbKeyboard;
  BleKeyboard bleKeyboard;
  KeyboardMode currentMode;
  bool isInitialized;
  bool usbAvailable;
  bool bluetoothAvailable;
  
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
  void setMode(KeyboardMode mode);
  KeyboardMode getCurrentMode();
  bool switchMode();  // モード切り替え（USB ⇔ Bluetooth）
  
  void sendShortcut(ShortcutCommand command);
  void sendKey(uint8_t key, uint8_t modifiers = 0);
  void sendKeyPress(uint8_t key, uint8_t modifiers = 0);
  void sendKeyRelease();
  
  bool isConnected();
  bool isUSBConnected();
  bool isBluetoothConnected();
  String getConnectionStatus();
  
private:
  uint8_t getKeyCode(String keyName);
  uint8_t getModifierCode(String keyName);
  void pressModifiers(uint8_t modifiers);
  void releaseModifiers(uint8_t modifiers);
  void delay_ms(int ms);
  
  // モード固有の送信処理
  void sendShortcutUSB(ShortcutCommand command);
  void sendShortcutBluetooth(ShortcutCommand command);
  void initializeUSB();
  void initializeBluetooth();
  void shutdownBluetooth();
};

#endif // KEYBOARDHANDLER_H