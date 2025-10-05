# EasyShortcutKey

🎹 **ショートカットキーを表示＆送信する統一システム**

ショートカットキーを覚える必要はもうありません！  
iPhone/iPad、ブラウザ、そしてWindows PCで、ショートカットキーを確認しながら実際にキー操作を送信できます。

## 🎯 特徴

- **📱 iOSアプリ**: ショートカットキー一覧表示＋実際のキー送信機能
- **💻 Windows常駐版 (ResidentWin)**: Windows PCで直接BLE接続、ハードウェア不要
- **🔌 ハードウェア版 (KeyboardGW)**: M5Stack AtomS3を使用したUSB HIDキーボード
- **🌐 ブラウザ版**: 設定ファイルを埋め込んだスタンドアロンHTML
- **📝 統一設定**: JSON形式で全プラットフォーム共通の設定管理

## 🚀 Version 3.0 - Windows常駐版リリース!

**新機能: Windows版KeyboardGW (ResidentWin)**

ハードウェア不要！Windows PC上で常駐するソフトウェア版KeyboardGWが登場！

```
📱 iPhone/iPad (iOSアプリ)
    ↓ Bluetooth Low Energy
💻 Windows PC (ResidentWin) ← NEW!
    ↓ キーボードエミュレーション
⌨️ 実際のキー入力
```

**従来のハードウェア版も引き続き利用可能:**

```
📱 iPhone/iPad (iOSアプリ)
    ↓ Bluetooth Low Energy
🔌 KeyboardGW (AtomS3ハードウェア)
    ↓ USB HID Keyboard
� PC (Windows/Mac)
```

## 📦 プラットフォーム

### 💻 ResidentWin - Windows常駐版 (NEW!)

Windows PCで常駐し、iPhoneからBLE経由でショートカットキーを受信してキーボード入力を実行します。

**特徴:**
- ✅ ハードウェア不要
- ✅ システムトレイに常駐
- ✅ BLE GATT Server実装
- ✅ 100種類以上のキー対応

**詳細:** [ResidentWin/README.md](ResidentWin/README.md)

**動作要件:**
- Windows 10 (Build 19041以上) または Windows 11
- BLE Peripheral Role対応Bluetoothアダプタ

### 📱 iOSアプリ

ショートカットキー一覧を表示し、タップでKeyboardGWにコマンドを送信します。

**特徴:**
- ✅ アプリ別ショートカット一覧
- ✅ カテゴリ別表示
- ✅ BLE接続管理
- ✅ ResidentWin / AtomS3 両対応

**詳細:** [iOS/README.md](iOS/README.md)

### 🔌 KeyboardGW - ハードウェア版

M5Stack AtomS3を使用したUSB HIDキーボードデバイス。

**特徴:**
- ✅ USB HID Keyboard
- ✅ BLE GATT Server
- ✅ Windows/Mac対応

**詳細:** [KeyboardGW/README.md](KeyboardGW/README.md)

### 🌐 ブラウザ版

設定ファイルを埋め込んだスタンドアロンHTML。参照専用。

**特徴:**
- ✅ オフライン動作
- ✅ 設定埋め込み
- ✅ 軽量（単一HTMLファイル）

**詳細:** [browser/README.md](browser/README.md)

## 🗂️ プロジェクト構成

```
EasyShortcutKey/
├── README.md                    # このファイル
├── config/                      # 設定ファイル
│   ├── shortcuts.json          # メイン設定
│   ├── schema.json             # JSONスキーマ
│   ├── shortcutJsons/          # アプリ別設定 (日本語)
│   └── shortcutJsons_en/       # アプリ別設定 (英語)
├── ResidentWin/                 # Windows常駐版 (NEW!)
│   ├── ResidentWin/            # メインアプリ
│   ├── BLETestConsole/         # BLE診断ツール
│   └── README.md
├── iOS/                         # iOSアプリ
│   ├── EasyShortcutKey/        # Xcodeプロジェクト
│   └── README.md
├── KeyboardGW/                  # AtomS3ファームウェア
│   ├── src/                    # ソースコード
│   ├── platformio.ini          # PlatformIO設定
│   └── README.md
├── browser/                     # ブラウザ版
│   ├── index.html              # 配布用HTML
│   ├── index.source.html       # ソースHTML
│   └── README.md
└── Manual/                      # マニュアル
    ├── Pairing.html            # ペアリング手順
    └── FirmwareFlash.html      # ファームウェア書き込み
```

## 🚀 クイックスタート

### Windows版を使う場合

1. **ResidentWinをビルド・実行**
   ```powershell
   cd ResidentWin/ResidentWin
   dotnet build
   .\bin\Debug\net9.0-windows10.0.19041.0\ResidentWin.exe
   ```

2. **iOSアプリから接続**
   - 設定 → KeyboardGW設定
   - デバイスを検索
   - PCの名前を選択して接続

3. **ショートカットを送信**
   - iOSアプリでショートカットをタップ
   - Windows PCで実行される

### ハードウェア版を使う場合

1. **AtomS3にファームウェアを書き込み**
   - 詳細: [Manual/FirmwareFlash.html](Manual/FirmwareFlash.html)

2. **PCに接続**
   - USB-CケーブルでPCと接続

3. **iOSアプリとペアリング**
   - 詳細: [Manual/Pairing.html](Manual/Pairing.html)

### ブラウザ版を使う場合

- `browser/index.html` をブラウザで開く
- ショートカット一覧を表示（参照専用）

## 📖 ドキュメント

- **使い方**: [USAGE.md](USAGE.md)
- **要件**: [REQUIREMENTS.md](REQUIREMENTS.md)
- **トラブルシューティング**: [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **ResidentWin詳細**: [ResidentWin/README.md](ResidentWin/README.md)
- **KeyboardGW詳細**: [KeyboardGW/README.md](KeyboardGW/README.md)
- **iOSアプリ詳細**: [iOS/README.md](iOS/README.md)

## 🔧 開発者向け情報

### 設定ファイル

- **メイン設定**: `config/shortcuts.json`
- **アプリ別設定**: `config/shortcutJsons/`
- **JSONスキーマ**: `config/schema.json`

### ビルド手順

- **ResidentWin**: `cd ResidentWin/ResidentWin && dotnet build`
- **iOSアプリ**: Xcodeで `iOS/EasyShortcutKey.xcodeproj` を開く
- **KeyboardGW**: `cd KeyboardGW && pio run`
- **ブラウザ版**: `cd browser && ./build_embed.zsh` (macOS) または `./build_embed.ps1` (Windows)

## 📝 ライセンス

MIT License

## 🙋 サポート

問題が発生した場合:
1. [TROUBLESHOOTING.md](TROUBLESHOOTING.md) を確認
2. 各プラットフォームのREADMEを確認
3. GitHubのIssuesで報告

---

**最終更新**: 2025年10月5日  
**Repository**: [NW-Lab/EasyShortcutKey](https://github.com/NW-Lab/EasyShortcutKey)


