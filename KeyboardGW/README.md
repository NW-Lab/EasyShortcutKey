# EasyShortcutKey KeyboardGW

このフォルダには、iPhone とPC間のショートカット中継を行うマイコン（KeyboardGW）のArduinoコードが含まれているよ。

## システム構成
```
iPhone —[BLE]→ KeyboardGW —[USB HID Keyboard]→ PC（Windows/Mac）
```

KeyboardGWは、iPhoneから受信したショートカットコマンドを、PC側にUSBキーボード入力として送信するデバイス。

## ハードウェア仕様
- **マイコン**: ESP32-S3（M5Stack AtomS3を使用）
- **通信**: BLE (GATT) + USB HID Keyboard
- **インジケータ**: 内蔵LEDで接続状態を表示
- **電源**: USB給電

## 機能要件

### BLE通信機能
- iPhone（Central）からの接続を受信（Peripheral役）
- GATTサービス・キャラクタリスティックによる通信
- セキュア接続（ペアリング済みデバイスのみ接続許可）
- 接続状態の管理と再接続処理

### USB HID Keyboard機能
- USB HIDデバイスとしてPC側に認識
- 修飾キー（Ctrl, Alt, Shift, Cmd等）の組み合わせ対応
- 複数キーの同時押し対応
- 日本語/英語キーボードレイアウト対応

### 状態表示機能
- 内蔵LEDによる動作状態表示
  - 起動中: 青点滅
  - BLE待機中: 青点灯
  - BLE接続済み: 緑点灯
  - キー送信中: 白点滅
  - エラー: 赤点滅

### セキュリティ機能
- ペアリング済みiPhoneのMACアドレス記録
- 未知デバイスからの接続拒否
- 接続タイムアウト処理

## 開発環境
- **IDE**: Arduino IDE または PlatformIO
- **ボードマネージャ**: ESP32 Arduino Core
- **必要ライブラリ**:
  - `ESP32 BLE Arduino`
  - `USB HID`
  - `M5AtomS3` (LED制御用)

## ファイル構成（予定）
```
KeyboardGW/
├── README.md              # このファイル
├── KeyboardGW.ino         # メインのArduinoスケッチ
├── BLEHandler.h/cpp       # BLE通信処理
├── KeyboardHandler.h/cpp  # USBキーボード処理
├── LEDIndicator.h/cpp     # LED表示制御
├── Config.h               # 設定定数・構造体定義
└── platformio.ini         # PlatformIO設定（オプション）
```

## プロトコル仕様

### BLE GATT構成
```
Service UUID: 12345678-1234-1234-1234-123456789ABC
├── Shortcut Command Characteristic (Write)
│   └── UUID: 12345678-1234-1234-1234-123456789ABD
├── Device Status Characteristic (Read/Notify)
│   └── UUID: 12345678-1234-1234-1234-123456789ABE
└── Pairing Info Characteristic (Write)
    └── UUID: 12345678-1234-1234-1234-123456789ABF
```

### コマンドフォーマット
ショートカットコマンドは以下のJSON形式で送信:
```json
{
  "keys": ["ctrl", "c"],
  "delay": 100
}
```

## ファームウェア書き込み方法
### ESPTool使用（配布用）
```bash
esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash -z 0x0 KeyboardGW.bin
```

### Arduino IDE使用（開発用）
1. ボード設定: `ESP32S3 Dev Module`
2. Partition Scheme: `Huge APP (3MB No OTA/1MB SPIFFS)`
3. USB Mode: `Hardware CDC and JTAG`
4. スケッチをアップロード

## 動作フロー
1. **起動**: LED青点滅、BLEアドバタイズ開始
2. **接続待機**: LED青点灯、iPhone からの接続を待機
3. **ペアリング**: 初回接続時のペアリング処理
4. **通信開始**: LED緑点灯、ショートカットコマンド受信待機
5. **キー送信**: コマンド受信時、対応するキー入力をPCに送信
6. **切断処理**: 接続切断時、再接続待機状態に戻る

## トラブルシューティング
- **PC側で認識されない**: USBケーブル確認、デバイスマネージャー確認
- **iPhone接続できない**: ペアリング情報リセット、Bluetooth設定確認
- **キー入力が効かない**: キーマッピング確認、レイアウト設定確認

## 変更履歴
- 初期プロジェクト作成
- 要件定義・仕様書作成