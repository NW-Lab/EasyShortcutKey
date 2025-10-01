// Minimal NimBLE-based KeyboardGW firmware
#include <Arduino.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>
#include "USB.h"
#include "Config.h"
#include "USBHID.h"
#include "LEDIndicator.h"

// Temporary debug: when set to 1, type debug information to the USB host via HID keyboard
// (useful for verifying what the iOS app actually sends in Notepad). Disable for normal operation.
#define DEBUG_TYPE_RAW 1

static NimBLECharacteristic* pShortcutChar = nullptr;
static NimBLECharacteristic* pStatusChar = nullptr;

#if DEBUG_TYPE_RAW
// Helper function to safely type debug strings via USB HID using Base64 encoding
void typeDebugString(const String& text) {
    const char* str = text.c_str();
    const char* arr[1] = { str };
    USBHID.writeKeys(arr, 1);
    delay(100); // Longer delay between debug outputs for readability
}

// Base64 encoding for safe transmission (basic implementation)
String encodeBase64(const String& input) {
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    String output = "";
    int val = 0, valb = -6;
    for (char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            output += chars[(val >> valb) & 0x3F];
            valb -= 6;
        }
    }
    if (valb > -6) output += chars[((val << 8) >> (valb + 8)) & 0x3F];
    while (output.length() % 4) output += '=';
    return output;
}

// Convert bytes to hex string safely
String bytesToHex(const std::string& data) {
    String hex = "";
    for (unsigned char c : data) {
        char buf[3];
        sprintf(buf, "%02x", c);
        hex += buf;
    }
    return hex;
}
#endif

class ShortcutCallbacks : public NimBLECharacteristicCallbacks {
private:
    std::string fragmentBuffer;
    uint32_t lastFragmentTime = 0;
    static const uint32_t FRAGMENT_TIMEOUT_MS = 1000;

public:
    void onWrite(NimBLECharacteristic* pCharacteristic) override {
        std::string value = pCharacteristic->getValue();
        uint32_t currentTime = millis();
        
        if (value.empty()) {
            if (pStatusChar) { pStatusChar->setValue("empty_payload"); pStatusChar->notify(); }
            return;
        }

        Serial.print("Received payload length: ");
        Serial.println(value.length());
        Serial.print("Payload hex: ");
        for (size_t i = 0; i < std::min(value.length(), (size_t)50); i++) {
            Serial.printf("%02x", (unsigned char)value[i]);
        }
        Serial.println();

#if DEBUG_TYPE_RAW
        // Type comprehensive debug info via USB HID for Notepad inspection
        // Use only hex digits (0-9, a-f) to avoid keyboard layout issues
        typeDebugString("\n");
        typeDebugString("dbg0len");
        typeDebugString(String(value.length()));
        typeDebugString("\n");
        typeDebugString("dbg0hex");
        typeDebugString(bytesToHex(value));
        typeDebugString("\n");
#endif
        
        // Check if this might be a fragment (timeout-based fragment detection)
        bool isFragment = false;
        if (!fragmentBuffer.empty() && (currentTime - lastFragmentTime) < FRAGMENT_TIMEOUT_MS) {
            isFragment = true;
            Serial.println("Detected continuation fragment");
        } else if (!fragmentBuffer.empty()) {
            Serial.println("Fragment timeout - clearing buffer");
            fragmentBuffer.clear();
        }
        
        // Handle potential fragmentation
        std::string completePayload;
        if (isFragment) {
            fragmentBuffer += value;
            completePayload = fragmentBuffer;
            Serial.print("Assembled fragment length: ");
            Serial.println(completePayload.length());
        } else {
            completePayload = value;
            fragmentBuffer = value; // Start new fragment sequence
        }
        
        lastFragmentTime = currentTime;
        
        // Try to parse as JSON
        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, completePayload);
        
        if (err) {
            Serial.print("JSON parse error: ");
            Serial.println(err.c_str());
            Serial.print("Raw payload (first 100 chars): ");
            Serial.println(completePayload.substr(0, 100).c_str());

#if DEBUG_TYPE_RAW
            typeDebugString("dbg1err\n");
            typeDebugString("dbg1hex");
            typeDebugString(bytesToHex(completePayload.substr(0, 50)));
            typeDebugString("\n");
#endif
            
            // If single fragment failed, wait for more fragments
            if (!isFragment && value.length() > 20) {
                Serial.println("Parse failed but payload looks incomplete - waiting for fragments");
#if DEBUG_TYPE_RAW
                typeDebugString("dbg2wait\n");
#endif
                if (pStatusChar) { pStatusChar->setValue("waiting_fragments"); pStatusChar->notify(); }
                return;
            }
            
            // Send detailed error info via status
            std::string errorMsg = "json_error:";
            errorMsg += err.c_str();
            if (pStatusChar) { 
                pStatusChar->setValue(errorMsg.substr(0, 100));
                pStatusChar->notify(); 
            }
            fragmentBuffer.clear(); // Clear on error
            return;
        }
        
        // JSON parsed successfully - clear fragment buffer and process
        fragmentBuffer.clear();
        Serial.println("JSON parsed successfully");

#if DEBUG_TYPE_RAW
        typeDebugString("dbg3ok\n");
        typeDebugString("dbg3hex");
        typeDebugString(bytesToHex(completePayload));
        typeDebugString("\n");
#endif
        
        if (pStatusChar) {
            // Send success status with payload length info
            std::string echo = "json_ok:len=" + std::to_string(completePayload.length());
            pStatusChar->setValue(echo);
            pStatusChar->notify();
        }

        if (!doc.containsKey("keys")) {
            Serial.println("No 'keys' field in JSON");
            if (pStatusChar) { pStatusChar->setValue("no_keys_field"); pStatusChar->notify(); }
            return;
        }

        JsonArray keys = doc["keys"].as<JsonArray>();
        Serial.print("Keys array size: ");
        Serial.println(keys.size());

#if DEBUG_TYPE_RAW
        typeDebugString("dbg4keycnt");
        typeDebugString(String(keys.size()));
        typeDebugString("\n");
#endif
        
        std::vector<String> keyStrings;
        keyStrings.reserve(keys.size());
        for (auto k : keys) {
            keyStrings.emplace_back(k.as<const char*>());
            Serial.print("Key: ");
            Serial.println(keyStrings.back().c_str());
#if DEBUG_TYPE_RAW
            typeDebugString("dbg4k");
            typeDebugString(String(keyStrings.size()-1));
            typeDebugString("hex");
            typeDebugString(bytesToHex(keyStrings.back().c_str()));
            typeDebugString("\n");
#endif
        }

#if DEBUG_TYPE_RAW
        typeDebugString("dbg9end\n\n");
        // Don't actually send keys in debug mode - just show what would be sent
        typeDebugString("dbgnokeys\n\n");
        LEDIndicator::blink(LED_WHITE, 80);
        if (pStatusChar) { pStatusChar->setValue("debug_complete"); pStatusChar->notify(); }
#else
        std::vector<const char*> keyPtrs;
        keyPtrs.reserve(keyStrings.size());
        for (auto &s : keyStrings) keyPtrs.push_back(s.c_str());

        USBHID.writeKeys(keyPtrs.data(), keyPtrs.size());

        // Indicate sending with a short white blink
        LEDIndicator::blink(LED_WHITE, 80);

        if (pStatusChar) { pStatusChar->setValue("keys_sent_ok"); pStatusChar->notify(); }
#endif
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

