# EasyShortcutKey — KeyboardGW (PlatformIO)# EasyShortcutKey KeyboardGW



このフォルダには、M5Stack AtomS3 (ESP32-S3) 用の KeyboardGW ファームウェア（PlatformIO ビルド）を置くよ。このフォルダには、iPhone とPC間のショートカット中継を行うマイコン（KeyboardGW）のArduinoコードが含まれているよ。



概要:## システム構成

```

- 役割: iPhone から受信したショートカットコマンドを USB HID Keyboard として PC に送信する中継デバイス。iPhone —[BLE]→ KeyboardGW —[USB HID Keyboard]→ PC（Windows/Mac）

- ハードウェア: M5Stack AtomS3 (ESP32-S3)```

- 通信: BLE (GATT Peripheral) と USB HID

KeyboardGWは、iPhoneから受信したショートカットコマンドを、PC側にUSBキーボード入力として送信するデバイス。

対応開発環境:

## ハードウェア仕様

- PlatformIO (推奨・唯一サポート)- **マイコン**: ESP32-S3（M5Stack AtomS3を使用）

- **通信**: BLE (GATT) + USB HID Keyboard

重要: 以前は Arduino IDE のサポートが書かれていたけど、現在は PlatformIO のみをサポートするようになったため、Arduino IDE に関する手順や記述は削除/無効化されているよ。- **インジケータ**: 内蔵LEDで接続状態を表示

- **電源**: USB給電

ファイル構成 (実装済み/主要ファイル):

## 機能要件

```

KeyboardGW/### BLE通信機能

├── README.md- iPhone（Central）からの接続を受信（Peripheral役）

├── platformio.ini- GATTサービス・キャラクタリスティックによる通信

├── src/- セキュア接続（ペアリング済みデバイスのみ接続許可）

│   ├── main.cpp- 接続状態の管理と再接続処理

│   ├── BLEHandler.cpp/h

│   ├── KeyboardHandler.cpp/h### USB HID Keyboard機能

│   ├── usb_descriptors.cpp- USB HIDデバイスとしてPC側に認識

│   └── ...- 修飾キー（Ctrl, Alt, Shift, Cmd等）の組み合わせ対応

├── firmware/               # 自動生成されるビルド成果物 (.bin)- WindowsのCopilotキーみたいに複合の場合の変換する。Copilotキーはwin+C

└── export_firmware.py      # ビルド後に firmware/ に .bin をコピーするスクリプト- 複数キーの同時押し対応

```- 日本語/英語キーボードレイアウト対応



ビルド方法 (開発者向け)### 状態表示機能

- 内蔵LEDによる動作状態表示

1. PlatformIO をインストール（VS Code + PlatformIO extension または pio CLI）  - 起動中: 青点滅

2. ターミナルでこのフォルダへ移動:  - BLE待機中: 青点灯

  - BLE接続済み: 緑点灯

```  - キー送信中: 白点滅

cd KeyboardGW  - エラー: 赤点滅

```

### セキュリティ機能

3. ビルド:- ペアリング済みiPhoneのMACアドレス記録

- 未知デバイスからの接続拒否

```- 接続タイムアウト処理

pio run

```## 開発環境

- **IDE**: PlatformIO

4. 書き込み (デバイスが接続されている場合):

## Version 2.0 対応キー一覧（実装済み）

```

pio run --target upload### 修飾キー（大文字小文字両対応）

```- `Ctrl`, `Control` → KEY_LEFT_CTRL

- `Shift` → KEY_LEFT_SHIFT  

ビルド後、`firmware/KeyboardGW_m5stack-atoms3.bin` が自動生成される（`export_firmware.py` によりコピーされる）。- `Alt`, `Option` → KEY_LEFT_ALT

- `Cmd`, `Command`, `Win`, `Windows` → KEY_LEFT_GUI

配布バイナリ書き込み (esptool.py)- 'copilot'  → KEY_LEFT_GUI + C



PlatformIO が使えない場合でも、配布済みの .bin を直接書き込む手順は以下の通り（参考）:### 文字・数字キー（A-Z、0-9）

- 大文字小文字両対応: `a`/`A`, `b`/`B`, ... `z`/`Z`

1. Python と esptool を用意:- 数字キー: `0`, `1`, `2`, ... `9`



```### ファンクションキー（大文字小文字両対応）

pip install esptool- `F1`/`f1`, `F2`/`f2`, ... `F12`/`f12`

```

### 特殊キー（大文字小文字両対応）

2. 接続ポート確認（Windows 例）:- `Enter`/`enter`, `Return`/`return` → KEY_RETURN

- `Space`/`space` → KEY_SPACE

```- `Tab`/`tab` → KEY_TAB

# PowerShell- `Backspace`/`backspace` → KEY_BACKSPACE

Get-WmiObject Win32_SerialPort | Select-Object DeviceID, Description- `Delete`/`delete` → KEY_DELETE

```- `Escape`/`escape`, `Esc`/`esc` → KEY_ESC



3. 書き込み例:### 矢印キー（記号と英語表記両対応）

- `↑`, `Up`/`up` → KEY_UP_ARROW

```- `↓`, `Down`/`down` → KEY_DOWN_ARROW  

esptool.py --chip esp32s3 --port COM3 --baud 921600 write_flash -z 0x0 firmware/KeyboardGW_m5stack-atoms3.bin- `←`, `Left`/`left` → KEY_LEFT_ARROW

```- `→`, `Right`/`right` → KEY_RIGHT_ARROW



注意点:### その他特殊キー

- `Home`/`home`, `End`/`end`

- このプロジェクトは現在 PlatformIO のみを公式サポートにしているよ。Arduino IDE に関する古い手順やコメントは無効化・削除済み。- `PageUp`/`pageup`, `PageDown`/`pagedown`

- 既存の古い実装や参考用コードは `KeyboardGW.old/` に残してあるので、必要なら参照してね。- `Insert`/`insert`



関連ドキュメント:### 記号キー

- ルート README.md- `-`, `=`, `[`, `]`, `\`, `;`, `'`, `,`, `.`, `/`, `` ` ``

- Manual/FirmwareFlash.html

- TROUBLESHOOTING.md## BLE通信プロトコル（実装済み）



変更履歴:### GATT Service & Characteristics

- KeyboardGW を PlatformIO のみに絞るため README を更新し、元の実装は `KeyboardGW.old/` に退避。- **Service UUID**: `12345678-1234-1234-1234-123456789ABC`

- **Shortcut Characteristic**: `12345678-1234-1234-1234-123456789ABD` (Write)
- **Status Characteristic**: `12345678-1234-1234-1234-123456789ABE` (Notify)  
- **Pairing Characteristic**: `12345678-1234-1234-1234-123456789ABF` (Write)
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
### ビルド (開発者)
PlatformIO:
```bash
cd KeyboardGW
pio run
```
ビルド後 `KeyboardGW/firmware/KeyboardGW_m5stack-atoms3.bin` が自動生成される（`export_firmware.py` post スクリプト）。

### 配布バイナリ書き込み (esptool.py)
1. Python と esptool を用意:
  ```bash
  pip install esptool
  ```
2. ポート確認 (macOS例):
  ```bash
  ls /dev/cu.usb* | grep -i modem
  ```
3. 既存 flash を消したい場合（任意）:
  ```bash
  esptool.py --chip esp32s3 --port /dev/cu.usbmodem1101 erase_flash
  ```
4. 書き込み:
  ```bash
  esptool.py --chip esp32s3 --port /dev/cu.usbmodem1101 --baud 921600 write_flash -z 0x0 firmware/KeyboardGW_m5stack-atoms3.bin
  ```
5. 再起動 (USB 再接続 or RST)。

Windows の場合は `/dev/cu.usbmodemXXXX` を `COMx` に置き換えてね。Baud は 921600 か 460800 が推奨。安定しない場合は 115200 に下げる。

### Arduino IDE使用（開発用）

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
- export_firmware スクリプト追加 (自動で firmware/.bin を生成)