#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// BLE設定
#define BLE_DEVICE_NAME "EasyShortcutKey-GW"
#define BLE_SERVICE_UUID "12345678-1234-1234-1234-123456789ABC"
#define BLE_SHORTCUT_CHAR_UUID "12345678-1234-1234-1234-123456789ABD"
#define BLE_STATUS_CHAR_UUID "12345678-1234-1234-1234-123456789ABE"
#define BLE_PAIRING_CHAR_UUID "12345678-1234-1234-1234-123456789ABF"

// LED設定（M5AtomS3）
#define LED_PIN 35
#define LED_COUNT 1

// LED色定義
#define LED_OFF 0x000000
#define LED_BLUE 0x0000FF
#define LED_GREEN 0x00FF00
#define LED_RED 0xFF0000
#define LED_WHITE 0xFFFFFF
#define LED_YELLOW 0xFFFF00

// システム設定
#define SERIAL_BAUD_RATE 115200
#define BLE_CONNECTION_TIMEOUT 30000  // 30秒
#define KEY_SEND_DELAY 50            // キー送信間隔(ms)

// デバイス状態定義
enum DeviceStatus {
  STATUS_STARTING = 0,
  STATUS_BLE_ADVERTISING,
  STATUS_BLE_CONNECTED,
  STATUS_SENDING_KEYS,
  STATUS_ERROR
};

// キーコード定義（USB HID用）
#define KEY_CTRL 0x01
#define KEY_SHIFT 0x02
#define KEY_ALT 0x04
#define KEY_GUI 0x08  // Windows key / Cmd key

// コマンド構造体
struct ShortcutCommand {
  String keys[8];  // 最大8キーの組み合わせ
  int keyCount;
  int delay;
};

// デバイス情報構造体
struct DeviceInfo {
  String macAddress;
  bool isPaired;
  unsigned long lastConnected;
};

#endif // CONFIG_H