// Copied from project root to src/ for PlatformIO include path
#ifndef BLEHANDLER_H
#define BLEHANDLER_H

#include "Config.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>

class BLEHandler : public BLEServerCallbacks, public BLECharacteristicCallbacks {
private:
  BLEServer* bleServer;
  BLEService* bleService;
  BLECharacteristic* shortcutCharacteristic;
  BLECharacteristic* statusCharacteristic;
  BLECharacteristic* pairingCharacteristic;
  
  bool deviceConnected;
  bool oldDeviceConnected;
  DeviceInfo pairedDevice;
  
  // コールバック関数ポインタ
  void (*onShortcutReceived)(ShortcutCommand command);
  void (*onConnectionChanged)(bool connected);

public:
  BLEHandler();
  void begin();
  void setShortcutCallback(void (*callback)(ShortcutCommand));
  void setConnectionCallback(void (*callback)(bool));
  void sendStatus(DeviceStatus status);
  void update();
  bool isConnected();
  
  // BLEServerCallbacks
  void onConnect(BLEServer* server) override;
  void onDisconnect(BLEServer* server) override;
  
  // BLECharacteristicCallbacks
  void onWrite(BLECharacteristic* characteristic) override;

private:
  void startAdvertising();
  void stopAdvertising();
  ShortcutCommand parseShortcutCommand(String jsonString);
  bool isDeviceAllowed(String macAddress);
  void savePairedDevice(String macAddress);
  void loadPairedDevice();
};

#endif // BLEHANDLER_H
