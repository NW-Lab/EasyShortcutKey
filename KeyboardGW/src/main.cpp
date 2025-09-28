// Minimal NimBLE-based KeyboardGW firmware
#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "Config.h"
#include "USBHID.h"

static NimBLECharacteristic* pShortcutChar = nullptr;
static NimBLECharacteristic* pStatusChar = nullptr;

class ShortcutCallbacks : public NimBLECharacteristicCallbacks {
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        if (value.empty()) return;

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

        if (pStatusChar) { pStatusChar->setValue("ok"); pStatusChar->notify(); }
    }
};

void setup() {
    Serial.begin(DEBUG_SERIAL_BAUD);
    delay(100);
    Serial.println("=== EasyShortcutKey KeyboardGW (PlatformIO) Starting ===");

    USBHID.begin();

    NimBLEDevice::init(DEVICE_NAME);
    NimBLEServer* pServer = NimBLEDevice::createServer();
    NimBLEService* pService = pServer->createService(SERVICE_UUID);

    pShortcutChar = pService->createCharacteristic(SHORTCUT_CHAR_UUID, NIMBLE_PROPERTY::WRITE);
    pShortcutChar->setCallbacks(new ShortcutCallbacks());

    pStatusChar = pService->createCharacteristic(STATUS_CHAR_UUID, NIMBLE_PROPERTY::NOTIFY);
    pStatusChar->addDescriptor(new NimBLE2902());

    pService->start();

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->start();

    Serial.println("BLE advertising started");
}

void loop() {
    delay(1000);
}
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
