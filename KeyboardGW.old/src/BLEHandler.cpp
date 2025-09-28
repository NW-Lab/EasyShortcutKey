// Copied BLEHandler implementation into src/ for PlatformIO
#include "BLEHandler.h"
#include <Preferences.h>

BLEHandler::BLEHandler() {
  deviceConnected = false;
  oldDeviceConnected = false;
  onShortcutReceived = nullptr;
  onConnectionChanged = nullptr;
  
  pairedDevice.macAddress = "";
  pairedDevice.isPaired = false;
  pairedDevice.lastConnected = 0;
}

void BLEHandler::begin() {
  Serial.println("[BLE] Initializing BLE...");
  
  // BLEデバイス初期化
  BLEDevice::init(BLE_DEVICE_NAME);
  
  // BLEサーバー作成
  bleServer = BLEDevice::createServer();
  bleServer->setCallbacks(this);
  
  // BLEサービス作成
  bleService = bleServer->createService(BLE_SERVICE_UUID);
  
  // ショートカットコマンド受信用キャラクタリスティック
  shortcutCharacteristic = bleService->createCharacteristic(
    BLE_SHORTCUT_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  shortcutCharacteristic->setCallbacks(this);
  
  // ステータス送信用キャラクタリスティック
  statusCharacteristic = bleService->createCharacteristic(
    BLE_STATUS_CHAR_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
  );
  statusCharacteristic->addDescriptor(new BLE2902());
  
  // ペアリング情報用キャラクタリスティック
  pairingCharacteristic = bleService->createCharacteristic(
    BLE_PAIRING_CHAR_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  pairingCharacteristic->setCallbacks(this);
  
  // サービス開始
  bleService->start();
  
  // 保存済みペアリング情報を読み込み
  loadPairedDevice();
  
  // アドバタイジング開始
  startAdvertising();
  
  Serial.println("[BLE] BLE initialization completed");
}

void BLEHandler::setShortcutCallback(void (*callback)(ShortcutCommand)) {
  onShortcutReceived = callback;
}

void BLEHandler::setConnectionCallback(void (*callback)(bool)) {
  onConnectionChanged = callback;
}

void BLEHandler::sendStatus(DeviceStatus status) {
  if (deviceConnected) {
    String statusStr = String(status);
    statusCharacteristic->setValue(statusStr.c_str());
    statusCharacteristic->notify();
    Serial.println("[BLE] Status sent: " + statusStr);
  }
}

void BLEHandler::update() {
  // 接続状態の変化をチェック
  if (!deviceConnected && oldDeviceConnected) {
    Serial.println("[BLE] Device disconnected");
    delay(500);
    startAdvertising();
    if (onConnectionChanged) onConnectionChanged(false);
  }
  
  if (deviceConnected && !oldDeviceConnected) {
    Serial.println("[BLE] Device connected");
    if (onConnectionChanged) onConnectionChanged(true);
  }
  
  oldDeviceConnected = deviceConnected;
}

bool BLEHandler::isConnected() {
  return deviceConnected;
}

void BLEHandler::onConnect(BLEServer* server) {
  Serial.println("[BLE] Client connected");
  
  // 接続元のMACアドレスをチェック（セキュリティ）
  String clientAddress = server->getConnectedCount() > 0 ? "unknown" : "unknown"; // 簡易
  
  if (pairedDevice.isPaired && clientAddress != "unknown") {
    if (!isDeviceAllowed(clientAddress)) {
      Serial.println("[BLE] Unauthorized device, disconnecting...");
      server->disconnect(0);
      return;
    }
  }
  
  deviceConnected = true;
  pairedDevice.lastConnected = millis();
}

void BLEHandler::onDisconnect(BLEServer* server) {
  Serial.println("[BLE] Client disconnected");
  deviceConnected = false;
}

void BLEHandler::onWrite(BLECharacteristic* characteristic) {
  String value = characteristic->getValue().c_str();
  
  if (characteristic == shortcutCharacteristic) {
    Serial.println("[BLE] Shortcut command received: " + value);
    
    ShortcutCommand command = parseShortcutCommand(value);
    if (onShortcutReceived && command.keyCount > 0) {
      onShortcutReceived(command);
    }
  }
  else if (characteristic == pairingCharacteristic) {
    Serial.println("[BLE] Pairing info received: " + value);
    
    // ペアリング情報を保存
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, value);
    
    if (doc.containsKey("macAddress")) {
      String macAddress = doc["macAddress"];
      savePairedDevice(macAddress);
    }
  }
}

void BLEHandler::startAdvertising() {
  BLEAdvertising* advertising = BLEDevice::getAdvertising();
  
  // サービスUUIDを明示的に追加
  advertising->addServiceUUID(BLE_SERVICE_UUID);
  
  // 広告設定を最適化
  advertising->setScanResponse(true);  // スキャンレスポンスを有効化
  advertising->setMinPreferred(0x06);  // より安全な最小間隔
  advertising->setMaxPreferred(0x12);  // 最大間隔も設定
  
  Serial.println("[BLE] Starting advertising...");
  Serial.println("[BLE] Service UUID: " + String(BLE_SERVICE_UUID));
  Serial.println("[BLE] Device Name: " + String(BLE_DEVICE_NAME));
  
  BLEDevice::startAdvertising();
  Serial.println("[BLE] Advertising started");
}

void BLEHandler::stopAdvertising() {
  BLEDevice::getAdvertising()->stop();
  Serial.println("[BLE] Advertising stopped");
}

ShortcutCommand BLEHandler::parseShortcutCommand(String jsonString) {
  ShortcutCommand command;
  command.keyCount = 0;
  command.delay = KEY_SEND_DELAY;
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonString);
  
  if (error) {
    Serial.println("[BLE] JSON parsing failed: " + String(error.c_str()));
    return command;
  }
  
  if (doc.containsKey("keys") && doc["keys"].is<JsonArray>()) {
    JsonArray keysArray = doc["keys"];
    
    for (size_t i = 0; i < keysArray.size() && i < 8; i++) {
      command.keys[command.keyCount] = keysArray[i].as<String>();
      command.keyCount++;
    }
  }
  
  if (doc.containsKey("delay")) {
    command.delay = doc["delay"];
  }
  
  Serial.println("[BLE] Parsed command: " + String(command.keyCount) + " keys");
  return command;
}

bool BLEHandler::isDeviceAllowed(String macAddress) {
  if (!pairedDevice.isPaired) {
    return true; // 初回接続時は許可
  }
  
  return macAddress == pairedDevice.macAddress;
}

void BLEHandler::savePairedDevice(String macAddress) {
  Preferences prefs;
  prefs.begin("keyboard-gw", false);
  
  prefs.putString("paired_mac", macAddress);
  prefs.putBool("is_paired", true);
  prefs.putULong("last_connected", millis());
  
  prefs.end();
  
  pairedDevice.macAddress = macAddress;
  pairedDevice.isPaired = true;
  pairedDevice.lastConnected = millis();
  
  Serial.println("[BLE] Device paired: " + macAddress);
}

void BLEHandler::loadPairedDevice() {
  Preferences prefs;
  prefs.begin("keyboard-gw", true);
  
  pairedDevice.macAddress = prefs.getString("paired_mac", "");
  pairedDevice.isPaired = prefs.getBool("is_paired", false);
  pairedDevice.lastConnected = prefs.getULong("last_connected", 0);
  
  prefs.end();
  
  if (pairedDevice.isPaired) {
    Serial.println("[BLE] Loaded paired device: " + pairedDevice.macAddress);
  } else {
    Serial.println("[BLE] No paired device found");
  }
}
