# EasyShortcutKey デバイス検出問題 - トラブルシューティングガイド

## 🔍 問題の診断結果

診断ツールの結果、以下の問題が判明しました：

### ❌ 検出された問題
- KeyboardGWデバイス（M5Stack AtomS3）がUSBデバイスとして認識されていません
- 関連するシリアルポートが見つかりません
- システムのBluetoothは正常ですが、デバイス自体が動作していない可能性があります

## 🔧 解決手順（優先度順）

### 1. 🔌 基本的な接続確認
**まず最初にチェックしてください：**

- [ ] KeyboardGWデバイスがUSBケーブルでPCに物理的に接続されているか
- [ ] USBケーブルがデータ通信に対応しているか（充電専用ではないか）
- [ ] 別のUSBポートで試してみる
- [ ] 別のUSBケーブルで試してみる

### 2. 💡 デバイスの状態確認
**LEDインジケーターをチェック：**

- **青色で点滅** → 起動中（正常）
- **青色で点灯** → Bluetooth待機中（正常・検出可能状態）
- **緑色で点灯** → 他のデバイスと接続済み
- **赤色** → エラー状態
- **消灯** → 電源未供給またはファームウェア異常 ⚠️

### 3. 🔄 デバイスリセット
**以下の手順でリセットを試してください：**

1. USBケーブルを抜いて10秒待機
2. 再度USBに接続
3. LEDが青色で点滅することを確認
4. 数秒後に青色で点灯に変わることを確認

### 4. 🛠 ファームウェアの確認・再書き込み
**LEDが消灯している場合：**

ファームウェアが正常に動作していない可能性があります。以下を実行してください：

#### Arduino IDE / PlatformIOを使用した確認方法：

1. **開発環境の準備**
   ```bash
   # PlatformIOの場合
   cd /Users/tauchi/github/EasyShortcutKey/KeyboardGW
   pio device list  # デバイスが認識されるかチェック
   
   # Arduino IDEの場合
   # ツール > ボード > ESP32 > M5Stack-ATOMS3 を選択
   # ツール > シリアルポート でデバイスが表示されるかチェック
   ```

2. **シリアルモニターでの確認**
   ```bash
   # PlatformIOの場合
   pio device monitor --baud 115200
   
   # または画面コマンド
   screen /dev/tty.usbserial-* 115200
   ```

3. **ファームウェア再書き込み**
   ```bash
   # PlatformIOの場合
   cd /Users/tauchi/github/EasyShortcutKey/KeyboardGW
   pio run --target upload
   
   # Arduino IDEの場合は「アップロード」ボタンをクリック
   ```

### 5. 📱 iOSアプリ側の確認
**デバイスが正常に動作している場合の確認方法：**

1. **iPhoneの設定アプリで確認**
   - 設定 > Bluetooth
   - "EasyShortcutKey-GW"という名前のデバイスが表示されるかチェック

2. **EasyShortcutKeyアプリで確認**
   - アプリの設定 > KeyboardGW
   - "デバイスを検索"をタップ
   - 発見されたデバイスリストを確認

## 🚨 緊急時の対応

### M5Stack AtomS3が完全に反応しない場合：

1. **ハードリセット**
   - デバイスのリセットボタンを長押し（存在する場合）
   - または完全に電源を切って再接続

2. **ブートローダーモードでの復旧**
   ```bash
   # ESP32のフラッシュ消去（最終手段）
   esptool.py --chip esp32s3 erase_flash
   
   # その後、ファームウェア再書き込み
   cd /Users/tauchi/github/EasyShortcutKey/KeyboardGW
   pio run --target upload
   ```

## 📞 サポート情報

### 正常な動作時の期待される動作：
1. USB接続時にデバイスがシリアルポートとして認識される
2. LEDが青色で点滅→点灯に変化
3. Bluetoothで"EasyShortcutKey-GW"として検出可能
4. iOSアプリから接続可能

### ログ確認コマンド：
```bash
# USBデバイス再確認
python3 /Users/tauchi/github/EasyShortcutKey/scripts/check_usb_device.py

# Bluetooth再確認  
python3 /Users/tauchi/github/EasyShortcutKey/scripts/debug_connection.py
```

---

この手順で解決しない場合は、ハードウェアの故障またはより深刻なファームウェアの問題の可能性があります。