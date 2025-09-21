#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// M5PaperS3固有設定
#define DEVICE_NAME "EasyShortcutKey-Paper"

// Bluetooth設定（キーボードモード用）
#define BT_DEVICE_NAME "EasyShortcutKey-Paper"
#define BT_DEVICE_MANUFACTURER "EasyShortcutKey"

// システム設定
#define SERIAL_BAUD_RATE 115200
#define KEY_SEND_DELAY 50

// 電源管理設定
#define POWER_BUTTON_PIN 37  // M5PaperS3の電源ボタン
#define POWER_LONG_PRESS_TIME 3000  // 3秒長押しでシャットダウン

// ディスプレイ設定
#define DISPLAY_WIDTH 540
#define DISPLAY_HEIGHT 960
#define DISPLAY_ROTATION 0

// レイアウト設定
#define LAYOUT_2_COLUMN 2
#define LAYOUT_3_COLUMN 3
#define DEFAULT_LAYOUT LAYOUT_2_COLUMN

// ボタンサイズ設定
#define BUTTON_WIDTH_2COL 250
#define BUTTON_HEIGHT_2COL 80
#define BUTTON_WIDTH_3COL 160
#define BUTTON_HEIGHT_3COL 80
#define BUTTON_MARGIN 10

// ヘッダー・フッター設定
#define HEADER_HEIGHT 60
#define FOOTER_HEIGHT 50
#define CONTENT_HEIGHT (DISPLAY_HEIGHT - HEADER_HEIGHT - FOOTER_HEIGHT)

// タッチ設定
#define TOUCH_THRESHOLD 40
#define LONG_PRESS_DURATION 1000  // ms
#define DOUBLE_TAP_INTERVAL 300   // ms

// 電力管理設定
#define AUTO_SLEEP_TIME 30000     // 30秒でスリープ
#define BATTERY_CHECK_INTERVAL 60000  // 1分間隔でバッテリーチェック
#define LOW_BATTERY_THRESHOLD 20  // 20%以下で警告

// ディスプレイ更新設定
#define UPDATE_MODE_FAST 0        // 高速更新（残像あり）
#define UPDATE_MODE_QUALITY 1     // 高品質更新（残像なし）
#define DEFAULT_UPDATE_MODE UPDATE_MODE_QUALITY

// データファイル設定
#define SHORTCUTS_FILE "/shortcuts.json"
#define CONFIG_FILE "/config.json"
#define SD_CS_PIN 4

// システム設定
#define SERIAL_BAUD_RATE 115200
#define KEY_SEND_DELAY 50

// フォント設定
#define FONT_SIZE_SMALL 16
#define FONT_SIZE_MEDIUM 20
#define FONT_SIZE_LARGE 24
#define FONT_SIZE_HEADER 28

// 色設定（e-ink用グレースケール）
#define COLOR_WHITE 15
#define COLOR_BLACK 0
#define COLOR_GRAY_LIGHT 12
#define COLOR_GRAY_MEDIUM 8
#define COLOR_GRAY_DARK 4

// デバイス状態定義
enum DeviceStatus {
  STATUS_STARTING = 0,
  STATUS_READY,
  STATUS_USB_MODE,
  STATUS_BLUETOOTH_MODE,
  STATUS_SENDING_KEYS,
  STATUS_SLEEPING,
  STATUS_LOW_BATTERY,
  STATUS_SHUTTING_DOWN,
  STATUS_ERROR
};

// キーボードモード定義
enum KeyboardMode {
  MODE_USB_HID = 0,
  MODE_BLUETOOTH,
  MODE_AUTO  // 自動選択（USB優先）
};

// 表示モード定義
enum DisplayMode {
  MODE_SHORTCUTS = 0,
  MODE_SETTINGS,
  MODE_BATTERY_INFO,
  MODE_ABOUT
};

// タッチイベント定義
enum TouchEvent {
  TOUCH_NONE = 0,
  TOUCH_TAP,
  TOUCH_LONG_PRESS,
  TOUCH_DOUBLE_TAP,
  TOUCH_SWIPE_LEFT,
  TOUCH_SWIPE_RIGHT,
  TOUCH_SWIPE_UP,
  TOUCH_SWIPE_DOWN
};

// ボタン構造体
struct Button {
  int x, y, width, height;
  String text;
  String description;
  String keys[8];  // ショートカットキー配列
  int keyCount;
  bool isVisible;
  bool isPressed;
  int id;
};

// ページ情報構造体
struct PageInfo {
  int currentPage;
  int totalPages;
  int buttonsPerPage;
  int totalButtons;
};

// システム設定構造体
struct SystemConfig {
  int layoutColumns;
  int updateMode;
  int autoSleepTime;
  int touchSensitivity;
  bool deepSleepEnabled;
  String dataSource;  // "internal" or "sdcard"
  KeyboardMode keyboardMode;  // USB/Bluetooth切り替え
  bool autoShutdownEnabled;
  int autoShutdownTime;  // 自動シャットダウン時間（分）
};

// バッテリー情報構造体
struct BatteryInfo {
  int percentage;
  float voltage;
  bool isCharging;
  bool isLowBattery;
  unsigned long lastUpdate;
};

// タッチ情報構造体
struct TouchInfo {
  int x, y;
  TouchEvent event;
  unsigned long timestamp;
  bool isValid;
};

// ショートカットコマンド構造体（KeyboardGWと共通）
struct ShortcutCommand {
  String keys[8];
  int keyCount;
  int delay;
};

#endif // CONFIG_H