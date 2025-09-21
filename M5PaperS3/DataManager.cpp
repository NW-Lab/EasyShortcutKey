#include "DataManager.h"

DataManager::DataManager() {
  sdCardAvailable = false;
  dataLoaded = false;
  currentDataSource = "internal";
  setDefaultConfig();
}

void DataManager::begin() {
  Serial.println("[Data] Initializing Data Manager...");
  
  // SDカード初期化を試行
  if (initSDCard()) {
    sdCardAvailable = true;
    Serial.println("[Data] SD card available");
    
    // SDカードからの読み込みを試行
    if (loadFromSDCard()) {
      currentDataSource = "sdcard";
      Serial.println("[Data] Loaded from SD card");
    } else {
      Serial.println("[Data] Failed to load from SD card, using internal data");
      loadFromInternal();
    }
  } else {
    Serial.println("[Data] SD card not available, using internal data");
    loadFromInternal();
  }
  
  // 設定ファイルの読み込み
  loadConfig();
  
  Serial.println("[Data] Data Manager initialized - Source: " + currentDataSource);
  Serial.println("[Data] Shortcuts loaded: " + String(shortcuts.size()));
}

bool DataManager::loadFromSDCard(const String& filename) {
  if (!sdCardAvailable) {
    Serial.println("[Data] SD card not available");
    return false;
  }
  
  if (!SD.exists(filename)) {
    Serial.println("[Data] File not found: " + filename);
    return false;
  }
  
  String jsonData = readFile(filename);
  if (jsonData.length() == 0) {
    Serial.println("[Data] Failed to read file: " + filename);
    return false;
  }
  
  return parseShortcutsJSON(jsonData);
}

bool DataManager::loadFromInternal() {
  Serial.println("[Data] Loading internal default shortcuts");
  currentDataSource = "internal";
  return parseShortcutsJSON(String(defaultShortcuts));
}

bool DataManager::loadConfig() {
  if (!sdCardAvailable || !fileExists(CONFIG_FILE)) {
    Serial.println("[Data] Config file not found, using defaults");
    setDefaultConfig();
    return false;
  }
  
  String configJson = readFile(CONFIG_FILE);
  if (configJson.length() == 0) {
    Serial.println("[Data] Failed to read config file");
    setDefaultConfig();
    return false;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, configJson);
  
  if (error) {
    Serial.println("[Data] Config JSON parsing failed: " + String(error.c_str()));
    setDefaultConfig();
    return false;
  }
  
  // 設定値を読み込み
  systemConfig.layoutColumns = doc["layoutColumns"] | DEFAULT_LAYOUT;
  systemConfig.updateMode = doc["updateMode"] | DEFAULT_UPDATE_MODE;
  systemConfig.autoSleepTime = doc["autoSleepTime"] | AUTO_SLEEP_TIME;
  systemConfig.touchSensitivity = doc["touchSensitivity"] | TOUCH_THRESHOLD;
  systemConfig.deepSleepEnabled = doc["deepSleepEnabled"] | true;
  systemConfig.dataSource = doc["dataSource"] | "internal";
  
  Serial.println("[Data] Config loaded successfully");
  return true;
}

void DataManager::saveConfig() {
  if (!sdCardAvailable) {
    Serial.println("[Data] Cannot save config - SD card not available");
    return;
  }
  
  DynamicJsonDocument doc(1024);
  doc["layoutColumns"] = systemConfig.layoutColumns;
  doc["updateMode"] = systemConfig.updateMode;
  doc["autoSleepTime"] = systemConfig.autoSleepTime;
  doc["touchSensitivity"] = systemConfig.touchSensitivity;
  doc["deepSleepEnabled"] = systemConfig.deepSleepEnabled;
  doc["dataSource"] = systemConfig.dataSource;
  
  String configJson;
  serializeJsonPretty(doc, configJson);
  
  if (writeFile(CONFIG_FILE, configJson)) {
    Serial.println("[Data] Config saved successfully");
  } else {
    Serial.println("[Data] Failed to save config");
  }
}

std::vector<Button> DataManager::getShortcuts() {
  return shortcuts;
}

bool DataManager::addShortcut(const Button& shortcut) {
  shortcuts.push_back(shortcut);
  Serial.println("[Data] Shortcut added: " + shortcut.text);
  return true;
}

bool DataManager::updateShortcut(int id, const Button& shortcut) {
  for (auto& btn : shortcuts) {
    if (btn.id == id) {
      btn = shortcut;
      btn.id = id;  // IDは保持
      Serial.println("[Data] Shortcut updated: ID " + String(id));
      return true;
    }
  }
  
  Serial.println("[Data] Shortcut not found for update: ID " + String(id));
  return false;
}

bool DataManager::removeShortcut(int id) {
  for (auto it = shortcuts.begin(); it != shortcuts.end(); ++it) {
    if (it->id == id) {
      shortcuts.erase(it);
      Serial.println("[Data] Shortcut removed: ID " + String(id));
      return true;
    }
  }
  
  Serial.println("[Data] Shortcut not found for removal: ID " + String(id));
  return false;
}

void DataManager::clearShortcuts() {
  shortcuts.clear();
  Serial.println("[Data] All shortcuts cleared");
}

String DataManager::getCurrentDataSource() {
  return currentDataSource;
}

void DataManager::setDataSource(const String& source) {
  if (source == "sdcard" && sdCardAvailable) {
    systemConfig.dataSource = source;
    loadFromSDCard();
  } else {
    systemConfig.dataSource = "internal";
    loadFromInternal();
  }
  currentDataSource = systemConfig.dataSource;
}

bool DataManager::isSDCardAvailable() {
  return sdCardAvailable;
}

void DataManager::refreshSDCard() {
  sdCardAvailable = initSDCard();
  Serial.println("[Data] SD card refresh: " + String(sdCardAvailable ? "Available" : "Not available"));
}

SystemConfig DataManager::getConfig() {
  return systemConfig;
}

void DataManager::setConfig(const SystemConfig& config) {
  systemConfig = config;
  Serial.println("[Data] Config updated");
}

String DataManager::exportToJSON() {
  DynamicJsonDocument doc(8192);
  
  doc["appName"] = "EasyShortcutKey M5PaperS3";
  doc["version"] = "1.0";
  doc["exportDate"] = "2025-09-21";
  
  JsonArray groups = doc.createNestedArray("groups");
  JsonObject group = groups.createNestedObject();
  group["name"] = "Exported Shortcuts";
  
  JsonArray shortcutsArray = group.createNestedArray("shortcuts");
  
  for (const auto& shortcut : shortcuts) {
    JsonObject shortcutObj = shortcutsArray.createNestedObject();
    shortcutObj["name"] = shortcut.text;
    shortcutObj["description"] = shortcut.description;
    
    JsonArray keysArray = shortcutObj.createNestedArray("keys");
    for (int i = 0; i < shortcut.keyCount; i++) {
      keysArray.add(shortcut.keys[i]);
    }
  }
  
  String jsonString;
  serializeJsonPretty(doc, jsonString);
  return jsonString;
}

bool DataManager::importFromJSON(const String& jsonData) {
  clearShortcuts();
  return parseShortcutsJSON(jsonData);
}

bool DataManager::saveToSDCard(const String& filename) {
  if (!sdCardAvailable) {
    Serial.println("[Data] Cannot save to SD card - not available");
    return false;
  }
  
  String jsonData = exportToJSON();
  if (writeFile(filename, jsonData)) {
    Serial.println("[Data] Data saved to SD card: " + filename);
    return true;
  }
  
  Serial.println("[Data] Failed to save to SD card: " + filename);
  return false;
}

bool DataManager::fileExists(const String& filename) {
  return sdCardAvailable && SD.exists(filename);
}

std::vector<String> DataManager::listFiles(const String& extension) {
  std::vector<String> files;
  
  if (!sdCardAvailable) {
    return files;
  }
  
  File root = SD.open("/");
  if (!root) {
    return files;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String filename = file.name();
      if (extension.length() == 0 || filename.endsWith(extension)) {
        files.push_back(filename);
      }
    }
    file = root.openNextFile();
  }
  
  root.close();
  return files;
}

bool DataManager::deleteFile(const String& filename) {
  if (!sdCardAvailable) {
    return false;
  }
  
  if (SD.remove(filename)) {
    Serial.println("[Data] File deleted: " + filename);
    return true;
  }
  
  Serial.println("[Data] Failed to delete file: " + filename);
  return false;
}

void DataManager::printShortcuts() {
  Serial.println("\n=== Shortcuts ===");
  for (const auto& shortcut : shortcuts) {
    Serial.print("ID: " + String(shortcut.id));
    Serial.print(" | Text: " + shortcut.text);
    Serial.print(" | Keys: " + formatKeysArray(shortcut));
    Serial.println(" | Desc: " + shortcut.description);
  }
  Serial.println("================\n");
}

void DataManager::printConfig() {
  Serial.println("\n=== Config ===");
  Serial.println("Layout Columns: " + String(systemConfig.layoutColumns));
  Serial.println("Update Mode: " + String(systemConfig.updateMode));
  Serial.println("Auto Sleep: " + String(systemConfig.autoSleepTime / 1000) + "s");
  Serial.println("Touch Sensitivity: " + String(systemConfig.touchSensitivity));
  Serial.println("Deep Sleep: " + String(systemConfig.deepSleepEnabled ? "Enabled" : "Disabled"));
  Serial.println("Data Source: " + systemConfig.dataSource);
  Serial.println("==============\n");
}

String DataManager::getDataInfo() {
  String info = "Data Source: " + currentDataSource + "\n";
  info += "Shortcuts: " + String(shortcuts.size()) + "\n";
  info += "SD Card: " + String(sdCardAvailable ? "Available" : "Not available") + "\n";
  return info;
}

bool DataManager::initSDCard() {
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("[Data] SD card initialization failed");
    return false;
  }
  
  Serial.println("[Data] SD card initialized successfully");
  return true;
}

bool DataManager::parseShortcutsJSON(const String& jsonData) {
  DynamicJsonDocument doc(8192);
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    Serial.println("[Data] JSON parsing failed: " + String(error.c_str()));
    return false;
  }
  
  clearShortcuts();
  
  // 新しい形式: 配列でアプリが分かれている
  if (doc.is<JsonArray>()) {
    JsonArray apps = doc.as<JsonArray>();
    int buttonId = 0;
    
    for (JsonObject app : apps) {
      if (app.containsKey("groups") && app["groups"].is<JsonArray>()) {
        JsonArray groups = app["groups"];
        
        for (JsonObject group : groups) {
          if (group.containsKey("shortcuts") && group["shortcuts"].is<JsonArray>()) {
            JsonArray shortcutsArray = group["shortcuts"];
            
            for (JsonObject shortcutObj : shortcutsArray) {
              Button button = createButtonFromJSON(shortcutObj, buttonId++);
              shortcuts.push_back(button);
            }
          }
        }
      }
    }
  }
  // 旧形式: オブジェクトで直接groupsを持つ
  else if (doc.containsKey("groups") && doc["groups"].is<JsonArray>()) {
    JsonArray groups = doc["groups"];
    int buttonId = 0;
    
    for (JsonObject group : groups) {
      if (group.containsKey("shortcuts") && group["shortcuts"].is<JsonArray>()) {
        JsonArray shortcutsArray = group["shortcuts"];
        
        for (JsonObject shortcutObj : shortcutsArray) {
          Button button = createButtonFromJSON(shortcutObj, buttonId++);
          shortcuts.push_back(button);
        }
      }
    }
  }
  
  dataLoaded = true;
  Serial.println("[Data] JSON parsed successfully, shortcuts: " + String(shortcuts.size()));
  return true;
}

Button DataManager::createButtonFromJSON(JsonObject shortcutObj, int id) {
  Button button;
  button.id = id;
  
  // 新形式では "action"、旧形式では "name"
  if (shortcutObj.containsKey("action")) {
    button.text = shortcutObj["action"].as<String>();
  } else {
    button.text = shortcutObj["name"] | "Unknown";
  }
  
  button.description = shortcutObj["description"] | "";
  button.isVisible = true;
  button.isPressed = false;
  button.keyCount = 0;
  
  if (shortcutObj.containsKey("keys") && shortcutObj["keys"].is<JsonArray>()) {
    JsonArray keysArray = shortcutObj["keys"];
    
    for (size_t i = 0; i < keysArray.size() && i < 8; i++) {
      button.keys[button.keyCount] = keysArray[i].as<String>();
      button.keyCount++;
    }
  }
  
  return button;
}

String DataManager::formatKeysArray(const Button& button) {
  String result = "";
  for (int i = 0; i < button.keyCount; i++) {
    if (i > 0) result += "+";
    result += button.keys[i];
  }
  return result;
}

void DataManager::setDefaultConfig() {
  systemConfig.layoutColumns = DEFAULT_LAYOUT;
  systemConfig.updateMode = DEFAULT_UPDATE_MODE;
  systemConfig.autoSleepTime = AUTO_SLEEP_TIME;
  systemConfig.touchSensitivity = TOUCH_THRESHOLD;
  systemConfig.deepSleepEnabled = true;
  systemConfig.dataSource = "internal";
}

String DataManager::readFile(const String& filename) {
  if (!sdCardAvailable) {
    return "";
  }
  
  File file = SD.open(filename);
  if (!file) {
    Serial.println("[Data] Failed to open file for reading: " + filename);
    return "";
  }
  
  String content = file.readString();
  file.close();
  
  return content;
}

bool DataManager::writeFile(const String& filename, const String& data) {
  if (!sdCardAvailable) {
    return false;
  }
  
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println("[Data] Failed to open file for writing: " + filename);
    return false;
  }
  
  size_t bytesWritten = file.print(data);
  file.close();
  
  return (bytesWritten == data.length());
}