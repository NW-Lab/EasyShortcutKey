// Minimal NimBLE-based KeyboardGW firmware
#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "USB.h"
#include "Config.h"
#include "USBHID.h"
#include "LEDIndicator.h"

// Temporary debug: when set to 1, type the raw received BLE payload to the USB host
// (useful for verifying what the iOS app actually sends in Notepad). Disable for normal operation.
#define DEBUG_TYPE_RAW 1

static NimBLECharacteristic* pShortcutChar = nullptr;
static NimBLECharacteristic* pStatusChar = nullptr;

class ShortcutCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        if (value.empty()) return;
        // Echo raw payload back via status characteristic for quick inspection on iOS
        if (pStatusChar) {
            // limit echo length to avoid large notifications; NimBLE can fragment but keep this small
            const size_t maxEcho = 200;
            std::string echo = value.substr(0, std::min(value.size(), maxEcho));
            pStatusChar->setValue(echo);
            pStatusChar->notify();
        }

#if DEBUG_TYPE_RAW
        // DEBUG: type hex dump (lowercase 0-9a-f) of payload so host layout doesn't mangle symbols
        {
            std::string hex;
            hex.reserve(value.size() * 2 + 1);
            const char* hexchars = "0123456789abcdef";
            for (unsigned char c : value) {
                hex.push_back(hexchars[(c >> 4) & 0xF]);
                hex.push_back(hexchars[c & 0xF]);
            }
            const char* out = hex.c_str();
            const char* arr[1] = { out };
            USBHID.writeKeys(arr, 1);
            LEDIndicator::blink(LED_WHITE, 80);
            if (pStatusChar) { pStatusChar->setValue("typed_hex"); pStatusChar->notify(); }
            return;
        }
#endif

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, value);
        if (err) {
            Serial.print("JSON parse error: ");
            Serial.println(err.c_str());
            if (pStatusChar) { pStatusChar->setValue("json_error"); pStatusChar->notify(); }
            return;
        }

        if (!doc.containsKey("keys")) {
            if (pStatusChar) { pStatusChar->setValue("no_keys"); pStatusChar->notify(); }
            return;
        }

        JsonArray keys = doc["keys"].as<JsonArray>();
        std::vector<String> keyStrings;
        keyStrings.reserve(keys.size());
        for (auto k : keys) {
            keyStrings.emplace_back(k.as<const char*>());
        }

        std::vector<const char*> keyPtrs;
        keyPtrs.reserve(keyStrings.size());
        for (auto &s : keyStrings) keyPtrs.push_back(s.c_str());

        USBHID.writeKeys(keyPtrs.data(), keyPtrs.size());

    // Indicate sending with a short white blink
    LEDIndicator::blink(LED_WHITE, 80);

        if (pStatusChar) { pStatusChar->setValue("ok"); pStatusChar->notify(); }
    }
};

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer) override {
        Serial.println("BLE connected");
        // Switch LED to green when a client connects
        LEDIndicator::setColor(LED_GREEN);
    }

    void onDisconnect(NimBLEServer* pServer) override {
        Serial.println("BLE disconnected");
        // Return to advertising color (blue) and restart advertising
        LEDIndicator::setColor(LED_BLUE);
        NimBLEDevice::getAdvertising()->start();
    }
};

void setup() {
    Serial.begin(DEBUG_SERIAL_BAUD);
    delay(100);
    Serial.println("=== EasyShortcutKey KeyboardGW (PlatformIO) Starting ===");

    USBHID.begin();
    // Ensure TinyUSB / USB stack is started so HID interface is enumerated
    USB.begin();
    // Initialize LED indicator and show startup sequence
    LEDIndicator::begin();
    // Startup: blink blue then steady blue (advertising)
    LEDIndicator::blink(LED_BLUE, 200);
    LEDIndicator::setColor(LED_BLUE);

    NimBLEDevice::init(DEVICE_NAME);
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks());
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pShortcutChar = pService->createCharacteristic(SHORTCUT_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
    pShortcutChar->setCallbacks(new ShortcutCallbacks());

    pStatusChar = pService->createCharacteristic(STATUS_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY);
    // Skip adding NimBLE2902 descriptor for compatibility across NimBLE-Arduino versions.
    // Initialize status characteristic value instead.
    pStatusChar->setValue("ready");

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE advertising started");
}

void loop() {
    delay(1000);
}

