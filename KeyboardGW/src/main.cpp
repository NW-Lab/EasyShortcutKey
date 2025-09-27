/*
 * EasyShortcutKey - KeyboardGW
 * ESP32-S3 AtomS3 + BLE + USB HID統合版
 */

#include <Arduino.h>
#include "tinyusb.h"
#include "tusb_cdc_acm.h"
#include "M5AtomS3.h"
#include "BLEDevice.h"
#include "BLEHandler.h"
#include "BLE2902.h"
#include "Config.h"
#include <esp_gap_ble_api.h>
#include <ArduinoJson.h>

// TinyUSB直接制御（参考サンプル準拠）
static const char *TAG = "KeyboardGW";

// CDC受信コールバック（参考サンプルから）
void tinyusb_cdc_rx_callback(int itf, cdcacm_event_t *event) {
    static uint8_t buf[CONFIG_TINYUSB_CDC_RX_BUFSIZE + 1];
    size_t rx_size = 0;
    esp_err_t ret = tinyusb_cdcacm_read(itf, buf, CONFIG_TINYUSB_CDC_RX_BUFSIZE, &rx_size);
    if (ret == ESP_OK && rx_size > 0) {
        ESP_LOGI(TAG, "CDC received %d bytes", rx_size);
        // HID送信（今は無効化）
        // if (rx_size > 0) {
        //     tud_hid_n_report(0, buf[0], &buf[1], rx_size - 1);
        // }
    }
}

// BLE設定
#define SERVICE_UUID        "12345678-1234-1234-1234-123456789abc"
#define CHARACTERISTIC_UUID "87654321-4321-4321-4321-cba987654321"

bool deviceConnected = false;

// BLEHandler をグローバル変数に
BLEHandler bleHandler;

// LED制御ヘルパー
inline void setDisplayColor(uint32_t color) {
    AtomS3.dis.drawpix(color);
    AtomS3.update();
}

// BLE handling is delegated to BLEHandler

// Shortcut handling will be done via BLEHandler callbacks

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.setDebugOutput(true);
    delay(500);

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
    
    // TinyUSB設定
    tinyusb_config_t tusb_cfg = {
        .device_descriptor = NULL,
        .string_descriptor = NULL,
        .string_descriptor_count = 0,
        .external_phy = false,
        .configuration_descriptor = NULL,
    };
    
    ESP_ERROR_CHECK(tinyusb_driver_install(&tusb_cfg));
    
    // CDC設定
    tinyusb_config_cdcacm_t acm_cfg = {
        .usb_dev = TINYUSB_USBDEV_0,
        .cdc_port = TINYUSB_CDC_ACM_0,
        .rx_unread_buf_sz = 64,
        .callback_rx = &tinyusb_cdc_rx_callback,
        .callback_rx_wanted_char = NULL,
    };
    ESP_ERROR_CHECK(tusb_cdc_acm_init(&acm_cfg));
    
    ESP_LOGI(TAG, "TinyUSB initialization DONE");
    Serial.println("TinyUSB Direct Control initialized!");
    
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
            
            // HID送信（今後実装予定）
            Serial.println("DEBUG: HID output not implemented in TinyUSB direct mode yet");
            
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
    
    delay(50);
}