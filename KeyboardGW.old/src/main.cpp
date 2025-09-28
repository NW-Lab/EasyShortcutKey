/*
 * EasyShortcutKey - KeyboardGW
 * ESP32-S3 AtomS3 + BLE + USB HID統合版
 */

#include <Arduino.h>
#include "tusb.h"
#include "M5AtomS3.h"
#include "BLEDevice.h"
#include "BLEHandler.h"
#include "BLE2902.h"
#include "Config.h"
#include <esp_gap_ble_api.h>
#include <ArduinoJson.h>

// HID Usage IDs for keyboard
#define HID_KEY_A 0x04
#define HID_KEY_B 0x05
#define HID_KEY_C 0x06
#define HID_KEY_D 0x07
#define HID_KEY_E 0x08
#define HID_KEY_F 0x09
#define HID_KEY_G 0x0A
#define HID_KEY_H 0x0B
#define HID_KEY_I 0x0C
#define HID_KEY_J 0x0D
#define HID_KEY_K 0x0E
#define HID_KEY_L 0x0F
#define HID_KEY_M 0x10
#define HID_KEY_N 0x11
#define HID_KEY_O 0x12
#define HID_KEY_P 0x13
#define HID_KEY_Q 0x14
#define HID_KEY_R 0x15
#define HID_KEY_S 0x16
#define HID_KEY_T 0x17
#define HID_KEY_U 0x18
#define HID_KEY_V 0x19
#define HID_KEY_W 0x1A
#define HID_KEY_X 0x1B
#define HID_KEY_Y 0x1C
#define HID_KEY_Z 0x1D
#define HID_KEY_1 0x1E
#define HID_KEY_2 0x1F
#define HID_KEY_3 0x20
#define HID_KEY_4 0x21
#define HID_KEY_5 0x22
#define HID_KEY_6 0x23
#define HID_KEY_7 0x24
#define HID_KEY_8 0x25
#define HID_KEY_9 0x26
#define HID_KEY_0 0x27
#define HID_KEY_RETURN 0x28
#define HID_KEY_ESCAPE 0x29
#define HID_KEY_BACKSPACE 0x2A
#define HID_KEY_TAB 0x2B
#define HID_KEY_SPACE 0x2C
#define HID_KEY_F1 0x3A
#define HID_KEY_F2 0x3B
#define HID_KEY_F3 0x3C
#define HID_KEY_F4 0x3D
#define HID_KEY_F5 0x3E
#define HID_KEY_F6 0x3F
#define HID_KEY_F7 0x40
#define HID_KEY_F8 0x41
#define HID_KEY_F9 0x42
#define HID_KEY_F10 0x43
#define HID_KEY_F11 0x44
#define HID_KEY_F12 0x45

// HID Modifier bits
#define HID_MOD_LCTRL 0x01
#define HID_MOD_LSHIFT 0x02
#define HID_MOD_LALT 0x04
#define HID_MOD_LGUI 0x08  // Left Windows/Cmd key
#define HID_MOD_RCTRL 0x10
#define HID_MOD_RSHIFT 0x20
#define HID_MOD_RALT 0x40
#define HID_MOD_RGUI 0x80  // Right Windows/Cmd key

// TinyUSB直接制御（参考サンプル準拠）
static const char *TAG = "KeyboardGW";

// We'll poll CDC in loop() using TinyUSB device API (tud_*).
// Declare minimal prototypes in case headers differ in this Arduino environment.
extern "C" {
    bool tud_hid_ready(void);
    bool tud_cdc_connected(void);
    size_t tud_cdc_read(void* buffer, uint32_t bufsize);
}

// TinyUSB CDC write helpers (declare minimally)
// Note: TinyUSB headers provide tud_cdc_write/tud_cdc_write_flush declarations.
// Avoid re-declaring them here to prevent signature conflicts.

// BLE設定
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

bool deviceConnected = false;

// BLEHandler をグローバル変数に
BLEHandler bleHandler;

// One-shot HID self-test flag
static bool hid_selftest_sent = false;

// LED制御ヘルパー
inline void setDisplayColor(uint32_t color) {
    AtomS3.dis.drawpix(color);
    AtomS3.update();
}

// キー文字列をHID Usage IDに変換
uint8_t getHIDKeyCode(String key) {
    key.toLowerCase();
    if (key == "a") return HID_KEY_A;
    if (key == "b") return HID_KEY_B;
    if (key == "c") return HID_KEY_C;
    if (key == "d") return HID_KEY_D;
    if (key == "e") return HID_KEY_E;
    if (key == "f") return HID_KEY_F;
    if (key == "g") return HID_KEY_G;
    if (key == "h") return HID_KEY_H;
    if (key == "i") return HID_KEY_I;
    if (key == "j") return HID_KEY_J;
    if (key == "k") return HID_KEY_K;
    if (key == "l") return HID_KEY_L;
    if (key == "m") return HID_KEY_M;
    if (key == "n") return HID_KEY_N;
    if (key == "o") return HID_KEY_O;
    if (key == "p") return HID_KEY_P;
    if (key == "q") return HID_KEY_Q;
    if (key == "r") return HID_KEY_R;
    if (key == "s") return HID_KEY_S;
    if (key == "t") return HID_KEY_T;
    if (key == "u") return HID_KEY_U;
    if (key == "v") return HID_KEY_V;
    if (key == "w") return HID_KEY_W;
    if (key == "x") return HID_KEY_X;
    if (key == "y") return HID_KEY_Y;
    if (key == "z") return HID_KEY_Z;
    if (key == "1") return HID_KEY_1;
    if (key == "2") return HID_KEY_2;
    if (key == "3") return HID_KEY_3;
    if (key == "4") return HID_KEY_4;
    if (key == "5") return HID_KEY_5;
    if (key == "6") return HID_KEY_6;
    if (key == "7") return HID_KEY_7;
    if (key == "8") return HID_KEY_8;
    if (key == "9") return HID_KEY_9;
    if (key == "0") return HID_KEY_0;
    if (key == "return" || key == "enter") return HID_KEY_RETURN;
    if (key == "escape" || key == "esc") return HID_KEY_ESCAPE;
    if (key == "backspace") return HID_KEY_BACKSPACE;
    if (key == "tab") return HID_KEY_TAB;
    if (key == "space") return HID_KEY_SPACE;
    if (key == "f1") return HID_KEY_F1;
    if (key == "f2") return HID_KEY_F2;
    if (key == "f3") return HID_KEY_F3;
    if (key == "f4") return HID_KEY_F4;
    if (key == "f5") return HID_KEY_F5;
    if (key == "f6") return HID_KEY_F6;
    if (key == "f7") return HID_KEY_F7;
    if (key == "f8") return HID_KEY_F8;
    if (key == "f9") return HID_KEY_F9;
    if (key == "f10") return HID_KEY_F10;
    if (key == "f11") return HID_KEY_F11;
    if (key == "f12") return HID_KEY_F12;
    return 0; // Unknown key
}

// 修飾キー文字列をHIDモディファイアビットに変換
uint8_t getHIDModifier(String mod) {
    mod.toLowerCase();
    if (mod == "ctrl" || mod == "control") return HID_MOD_LCTRL;
    if (mod == "shift") return HID_MOD_LSHIFT;
    if (mod == "alt" || mod == "option") return HID_MOD_LALT;
    if (mod == "cmd" || mod == "gui" || mod == "win") return HID_MOD_LGUI;
    if (mod == "rctrl") return HID_MOD_RCTRL;
    if (mod == "rshift") return HID_MOD_RSHIFT;
    if (mod == "ralt") return HID_MOD_RALT;
    if (mod == "rcmd" || mod == "rgui" || mod == "rwin") return HID_MOD_RGUI;
    return 0; // Unknown modifier
}

// BLE受信キーをHIDレポートに変換して送信
void sendHIDKeys(String keyString) {
    Serial.println("Converting to HID: " + keyString);
    
    // HIDレポート初期化 [modifier, reserved, key1, key2, key3, key4, key5, key6]
    uint8_t report[8] = {0};
    uint8_t keyIndex = 2; // キーは2番目から
    
    // "+"で分割してキーを解析
    String currentKey = "";
    for (int i = 0; i <= keyString.length(); i++) {
        if (i == keyString.length() || keyString.charAt(i) == '+') {
            if (currentKey.length() > 0) {
                currentKey.trim();
                
                // 修飾キーかチェック
                uint8_t modifier = getHIDModifier(currentKey);
                if (modifier != 0) {
                    report[0] |= modifier; // 修飾キーを追加
                    Serial.println("Modifier: " + currentKey + " (0x" + String(modifier, HEX) + ")");
                } else {
                    // 通常キー
                    uint8_t keyCode = getHIDKeyCode(currentKey);
                    if (keyCode != 0 && keyIndex < 8) {
                        report[keyIndex] = keyCode;
                        Serial.println("Key: " + currentKey + " (0x" + String(keyCode, HEX) + ")");
                        keyIndex++;
                    } else {
                        Serial.println("Unknown key: " + currentKey);
                    }
                }
            }
            currentKey = "";
        } else {
            currentKey += keyString.charAt(i);
        }
    }
    
    // HIDレポート送信
    #if defined(tud_hid_ready) && defined(tud_hid_report)
    if (tud_hid_ready()) {
        Serial.print("Sending HID report: ");
        for (int i = 0; i < 8; i++) {
            Serial.print("0x" + String(report[i], HEX) + " ");
        }
        Serial.println();
        
        // キー押下
        tud_hid_report(0, report, sizeof(report));
        delay(50);
        
        // キー離す
        memset(report, 0, sizeof(report));
        tud_hid_report(0, report, sizeof(report));
        
        Serial.println("HID report sent successfully");
    } else {
        Serial.println("HID not ready");
    }
    #else
    Serial.println("HID functions not available");
    #endif
}

// BLE handling is delegated to BLEHandler

// Shortcut handling will be done via BLEHandler callbacks

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setDebugOutput(true);
    delay(500);

    // Force some early debug prints to ensure UART is working
    Serial.println("=== BOOT SEQUENCE START ===");
    Serial.println("Serial test: 1");
    delay(50);
    Serial.println("Serial test: 2");
    delay(50);
    Serial.println("Serial test: 3");

    // Also attempt to print the same messages over TinyUSB CDC if available
    // Use runtime check so code is always compiled; relies on TinyUSB headers
    if (tud_cdc_connected()) {
        const char* s1 = "=== BOOT SEQUENCE START (CDC) ===\r\n";
        tud_cdc_write((const uint8_t*)s1, strlen(s1));
        tud_cdc_write_flush();
        const char* s2 = "Serial test (CDC): OK\r\n";
        tud_cdc_write((const uint8_t*)s2, strlen(s2));
        tud_cdc_write_flush();
    }

    // M5AtomS3初期化
    AtomS3.begin(true);
    AtomS3.dis.setBrightness(100);

    Serial.println("=== EasyShortcutKey KeyboardGW Starting ===");

    // BLE接続状態を明示的に初期化
    deviceConnected = false;

    // ディスプレイ初期化（赤色 = 初期化中）
    setDisplayColor(0xff0000);
    Serial.println("Display: RED (Initializing)");

    Serial.println("Initializing TinyUSB Direct Control...");

    // TinyUSB直接初期化（参考サンプル準拠）
    ESP_LOGI(TAG, "USB initialization");

    // Note: in the Arduino/PlatformIO environment the TinyUSB device
    // stack is provided by the core. Avoid using IDF-specific
    // tinyusb_driver_install / tusb_cdc_acm APIs which are not
    // available here. We'll rely on the tud_* (TinyUSB Device) APIs
    // and poll USB in loop().
    ESP_LOGI(TAG, "TinyUSB (Arduino integration) - using tud_* API if available");
    Serial.println("TinyUSB (Arduino integration) initialized (no explicit driver install)");
    // Log current HID ready state for debugging
    bool hid_ready = false;
    // Some Arduino TinyUSB integrations expose tud_hid_ready; if available, call it
    #if defined(tud_hid_ready)
    hid_ready = tud_hid_ready();
    #endif
    Serial.printf("HID ready: %s\n", hid_ready ? "yes" : "no");
    
    // BLEHandler に初期化を委譲
    Serial.println("Initializing BLE via BLEHandler...");
    bleHandler.begin();

    // ショートカット受信時のコールバックを登録
    bleHandler.setShortcutCallback([&bleHandler](ShortcutCommand cmd){
        Serial.println("=== ショートカット受信 ===");
        Serial.println("keyCount: " + String(cmd.keyCount));
        
        // 実行中はオレンジ
        setDisplayColor(0xff8000);  // オレンジ色
        
        if (cmd.keyCount > 0) {
            Serial.println("Received keys:");
            for (int i = 0; i < cmd.keyCount; ++i) {
                Serial.println("  [" + String(i) + "] = '" + cmd.keys[i] + "'");
            }
            
            // デバッグ用：受信したデータをそのまま表示（TinyUSB版）
            Serial.println("DEBUG: Raw key data received (TinyUSB mode)");
            // keyboard.print は使用不可（TinyUSB直接制御のため）
            Serial.print("RECEIVED_KEYS:");
            for (int i = 0; i < cmd.keyCount; ++i) {
                Serial.print(cmd.keys[i]);
                if (i < cmd.keyCount - 1) {
                    Serial.print("+");
                }
            }
            Serial.println(" ");
            
            // HID送信実装
            Serial.println("DEBUG: Converting BLE keys to HID output");
            for (int i = 0; i < cmd.keyCount; ++i) {
                String keyString = String(cmd.keys[i]);
                Serial.println("Processing key: " + keyString);
                sendHIDKeys(keyString);
                if (i < cmd.keyCount - 1) {
                    delay(100); // 複数キーの間隔
                }
            }
            
            Serial.println("=== ショートカット送信完了 ===");
        } else {
            Serial.println("No keys to send");
            // keyboard.print は使用不可
            Serial.println("NO_KEYS_RECEIVED");
        }
        
        delay(100);
        setDisplayColor(bleHandler.isConnected() ? 0x00ff00 : 0x0000ff);
    });

    // 接続状態変化のコールバック
    bleHandler.setConnectionCallback([](bool connected){
        if (connected) {
            setDisplayColor(0x00ff00);
            Serial.println("Display: GREEN (BLE Connected)");
        } else {
            setDisplayColor(0x0000ff);
            Serial.println("Display: BLUE (BLE Disconnected, advertising restarted)");
        }
    });
    
    // 青色点灯（待機状態）
    setDisplayColor(0x0000ff);  // 青色（16進数）
    Serial.println("Display: BLUE (BLE Advertising, waiting for connection)");
    
    // 初期テスト（5秒後）
    //delay(5000);
    //Serial.println("Initial test: sending 'KeyboardGW Ready!'");
    //keyboard.print("KeyboardGW Ready!");
    //keyboard.write(KEY_RETURN);
    
    Serial.println("=== Setup completed ===");
}

void loop() {
    M5.update();
    // Allow TinyUSB device stack to run
    #if defined(tud_task)
    tud_task();
    #endif
    
    // BLE接続状態の更新処理
    bleHandler.update();
    
    // ボタンが押されたらテスト送信（TinyUSB版）
    if (M5.BtnA.wasPressed()) {
        Serial.println("Button pressed - TinyUSB direct mode test");
        // keyboard.print は使用不可
        Serial.println("Button Test from AtomS3 (TinyUSB mode)");
        
        // オレンジ色に一瞬点灯（ボタンテスト）
        setDisplayColor(0xff8000);  // オレンジ色（16進数）
        delay(200);
        
        // 元の色に戻す
        if (deviceConnected) {
            setDisplayColor(0x00ff00);  // 緑色（16進数）
        } else {
            setDisplayColor(0x0000ff);  // 青色（16進数）
        }
    }
    
    // Startup HID self-test: send one 'a' key report to host if HID is ready
    if (!hid_selftest_sent) {
        bool hid_ready = false;
        #if defined(tud_hid_ready)
        hid_ready = tud_hid_ready();
        #endif

        if (hid_ready) {
            // Visual cue: flash orange
            setDisplayColor(0xff8000);
            delay(100);

            // HID keyboard report: [modifier, reserved, keycode1, keycode2...]
            uint8_t report[8] = {0};
            // HID usage ID for 'a' is 0x04
            report[2] = 0x04;

            #if defined(tud_hid_report)
            tud_hid_report(0, report, sizeof(report));
            // release
            memset(report, 0, sizeof(report));
            tud_hid_report(0, report, sizeof(report));
            #endif

            // restore color based on BLE connection
            setDisplayColor(bleHandler.isConnected() ? 0x00ff00 : 0x0000ff);
            hid_selftest_sent = true;
        }
    }

    // Periodic heartbeat to help debug CDC/Serial connectivity
    static uint32_t last_hb = 0;
    if (millis() - last_hb > 1000) {
        last_hb = millis();
        Serial.println("HEARTBEAT: Serial alive");
        #if defined(tud_cdc_connected)
        if (tud_cdc_connected()) {
            const char* hb = "HEARTBEAT: CDC alive\r\n";
            tud_cdc_write((const uint8_t*)hb, strlen(hb));
            tud_cdc_write_flush();
        }
        #endif
    }

    delay(50);
}
