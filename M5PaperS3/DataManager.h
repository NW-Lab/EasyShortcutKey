#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "Config.h"
#include <SD.h>
#include <ArduinoJson.h>
#include <vector>

class DataManager {
private:
  std::vector<Button> shortcuts;
  SystemConfig systemConfig;
  bool sdCardAvailable;
  bool dataLoaded;
  String currentDataSource;
  
  // 内蔵データ
  const char* defaultShortcuts = R"([
    {
      "appName": "Default Shortcuts",
      "order": 1,
      "groups": [
        {
          "groupName": "Basic",
          "order": 1,
          "shortcuts": [
            {"action": "Copy", "keys": ["ctrl", "c"], "description": "Copy selected", "order": 1},
            {"action": "Paste", "keys": ["ctrl", "v"], "description": "Paste clipboard", "order": 2},
            {"action": "Undo", "keys": ["ctrl", "z"], "description": "Undo last action", "order": 3},
            {"action": "Redo", "keys": ["ctrl", "y"], "description": "Redo action", "order": 4},
            {"action": "Select All", "keys": ["ctrl", "a"], "description": "Select all", "order": 5},
            {"action": "Save", "keys": ["ctrl", "s"], "description": "Save file", "order": 6}
          ]
        },
        {
          "groupName": "Navigation",
          "order": 2,
          "shortcuts": [
            {"action": "Find", "keys": ["ctrl", "f"], "description": "Find text", "order": 1},
            {"action": "Replace", "keys": ["ctrl", "h"], "description": "Replace text", "order": 2},
            {"action": "New Tab", "keys": ["ctrl", "t"], "description": "Open new tab", "order": 3},
            {"action": "Close Tab", "keys": ["ctrl", "w"], "description": "Close current tab", "order": 4}
          ]
        }
      ]
    }
  ])";

public:
  DataManager();
  void begin();
  
  // データ読み込み
  bool loadFromSDCard(const String& filename = SHORTCUTS_FILE);
  bool loadFromInternal();
  bool loadConfig();
  void saveConfig();
  
  // ショートカット管理
  std::vector<Button> getShortcuts();
  bool addShortcut(const Button& shortcut);
  bool updateShortcut(int id, const Button& shortcut);
  bool removeShortcut(int id);
  void clearShortcuts();
  
  // データソース管理
  String getCurrentDataSource();
  void setDataSource(const String& source);
  bool isSDCardAvailable();
  void refreshSDCard();
  
  // 設定管理
  SystemConfig getConfig();
  void setConfig(const SystemConfig& config);
  
  // データエクスポート/インポート
  String exportToJSON();
  bool importFromJSON(const String& jsonData);
  bool saveToSDCard(const String& filename = SHORTCUTS_FILE);
  
  // ファイル操作
  bool fileExists(const String& filename);
  std::vector<String> listFiles(const String& extension = ".json");
  bool deleteFile(const String& filename);
  
  // デバッグ情報
  void printShortcuts();
  void printConfig();
  String getDataInfo();

private:
  bool initSDCard();
  bool parseShortcutsJSON(const String& jsonData);
  Button createButtonFromJSON(JsonObject shortcutObj, int id);
  String formatKeysArray(const Button& button);
  void setDefaultConfig();
  String readFile(const String& filename);
  bool writeFile(const String& filename, const String& data);
};

#endif // DATAMANAGER_H