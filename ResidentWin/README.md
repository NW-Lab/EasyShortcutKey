# ResidentWin - Windows版 KeyboardGW

## 概要

Windows上で常駐してBLE GATT Serverとして動作し、iPhoneのEasyShortcutKeyアプリからショートカットキーを受信してキーボード入力をエミュレートするアプリケーションです。

AtomS3ハードウェア版のKeyboardGWと同じ機能をWindows PCで実現します。

## システム構成

```
iPhone —[BLE]→ ResidentWin (Windows常駐アプリ) —[仮想キーボード入力]→ Windows アプリケーション
```

### KeyboardGW(ハードウェア版)との違い

| 項目 | KeyboardGW | ResidentWin |
|------|-----------|-------------|
| 実装環境 | ESP32-S3 (PlatformIO) | Windows (C#/.NET) |
| 通信 | USB HID + BLE | BLE |
| キー入力 | USB HIDキーボード | Win32 API (SendInput) |
| 電源 | USB給電 | PC電源 |
| 配置 | 外付けデバイス | 常駐ソフトウェア |

## 技術スタック

### 推奨実装: C# + .NET Framework/Core

- **言語**: C# 
- **フレームワーク**: .NET 6.0 以上 (クロスプラットフォーム対応)
- **BLE通信**: Windows.Devices.Bluetooth API
- **キーボード入力**: Win32 API (SendInput) via P/Invoke
- **UI**: WPF または Windows Forms (システムトレイアプリ)
- **IDE**: Visual Studio 2022 Community 以上

### 代替実装案

1. **Python**
   - BLE: `bleak`
   - キーボード: `pynput` または `pyautogui`
   - UI: `pystray` (システムトレイ)
   
2. **C++**
   - BLE: WinRT API (C++/WinRT)
   - キーボード: Win32 API 直接呼び出し

## 実装済み機能

### 1. BLE通信機能

- ✅ iPhone (Central) からの接続受信 (Peripheral役)
- ✅ GATTサービス・キャラクタリスティック実装
- ✅ 接続状態管理
- ✅ BLE UUIDはKeyboardGWと共通

#### GATT構成 (KeyboardGWと同じ)

```
Service UUID: 12345678-1234-1234-1234-123456789ABC
  ├─ Characteristic (Shortcut): 12345678-1234-1234-1234-123456789ABD
  │    Properties: WRITE, NOTIFY
  │    受信: JSON形式のショートカットコマンド
  └─ Characteristic (Status): 12345678-1234-1234-1234-123456789ABE
       Properties: READ, NOTIFY
       送信: 接続状態・ステータス通知
```

### 2. キーボード入力エミュレーション

- ✅ Win32 SendInput APIによる仮想キー入力
- ✅ 修飾キー対応 (Ctrl, Alt, Shift, Win, Copilot等)
- ✅ 複数キー同時押し対応
- ✅ CopilotキーなどのWindows特殊キー変換
  - `Copilot` → `Win + C`
  - 100種類以上のキーマッピング対応
- ✅ 日本語/英語キーボードレイアウト対応

### 3. 常駐アプリケーション機能

- ✅ システムトレイアイコン表示
- ✅ 右クリックメニュー
  - デバイス名表示
  - 終了
- ⏳ Windows起動時の自動起動 (TODO)
- ⏳ トースト通知 (TODO)

### 4. 設定・管理機能

- ✅ ログ出力 (デバッグモード)
- ✅ 設定ファイル管理
- ⏳ ペアリング済みデバイス管理 (TODO)
- ⏳ ショートカット設定読み込み (`shortcuts.json`) (TODO)

## ファイル構成 (予定)

```
ResidentWin/
├── README.md                      # このファイル
├── ResidentWin.sln               # Visual Studio ソリューション
├── ResidentWin/                  # メインアプリケーション
│   ├── ResidentWin.csproj       # プロジェクトファイル
│   ├── App.xaml                 # WPFアプリケーション定義
│   ├── App.xaml.cs              # エントリーポイント
│   └── src/
│       ├── BLE/
│       │   └── BLEManager.cs           # BLE GATT Server実装
│       ├── Keyboard/
│       │   ├── KeyboardEmulator.cs     # Win32 SendInput実装
│       │   └── KeyMapping.cs           # キーマッピング定義 (100+種類)
│       ├── Models/
│       │   ├── ShortcutCommand.cs      # ショートカットコマンドモデル
│       │   └── ConnectionState.cs      # 接続状態Enum
│       ├── UI/
│       │   └── TrayIconManager.cs      # システムトレイ管理
│       └── Utils/
│           ├── Logger.cs               # ログ出力
│           ├── ConfigManager.cs        # 設定管理
│           └── BLETest.cs              # BLE診断テスト
└── BLETestConsole/               # BLE機能テストツール
    ├── BLETestConsole.csproj
    ├── Program.cs                # コンソールアプリ
    └── README.md
```

## システム要件

- **OS**: Windows 10 (Build 19041以上) または Windows 11
- **.NET**: .NET 9.0 Runtime
- **Bluetooth**: BLE Peripheral Role対応のBluetoothアダプタ
  - ⚠️ 多くのBluetoothアダプタはBLE Peripheral (GATT Server) をサポートしていません
  - Intel Wireless Bluetooth、Realtek など一部のチップセットのみ対応
  - 外付けの場合、「BLE Peripheral」または「BLE Server」対応を確認してください

## インストールと実行

### 開発環境のセットアップ

1. .NET 9.0 SDK をインストール
2. Visual Studio 2022 または VS Code をインストール

### ビルド

```powershell
cd ResidentWin/ResidentWin
dotnet build
```

### 実行

```powershell
cd ResidentWin/ResidentWin/bin/Debug/net9.0-windows10.0.19041.0
.\ResidentWin.exe
```

### パブリッシュ (配布用)

```powershell
dotnet publish ResidentWin/ResidentWin/ResidentWin.csproj `
  --configuration Release `
  --runtime win-x64 `
  --self-contained true `
  --output ./publish
```

## 使い方

1. **ResidentWin.exeを起動**
   - タスクトレイにアイコンが表示されます
   - 自動的にBLEアドバタイズが開始されます

2. **iPhoneでEasyShortcutKeyアプリを起動**
   - 設定画面からKeyboardGWペアリング画面を開く
   - 「デバイスを検索」をタップ

3. **デバイスを選択**
   - Windows PCの名前 (例: "HP"、"DESKTOP-XXX") が表示されます
   - タップして接続

4. **ショートカットキー送信**
   - iPhoneアプリからショートカットキーを送信
   - Windows PCで実際のキーボード入力として実行されます

## テスト・デバッグ

### BLE機能テスト

Bluetoothアダプタの機能を確認:

```powershell
cd ResidentWin/BLETestConsole
dotnet run
```

出力の確認項目:
```
IsPeripheralRoleSupported: True  ← これがFalseの場合は非対応
```

### ログ確認

ログは以下に出力されます:
```
%LOCALAPPDATA%\ResidentWin\logs\ResidentWin.log
```

## セキュリティ考慮事項

### 管理者権限

一部のアプリケーション (管理者として実行されているもの) にキー入力を送るには、ResidentWin自体も管理者権限で実行する必要がある。

**対応方法**:
- アプリケーションマニフェストで `requireAdministrator` を設定
- または、ユーザーに右クリック → "管理者として実行" を案内

### BLEセキュリティ

- ペアリング済みデバイスのみ接続許可
- MACアドレスのホワイトリスト管理
- タイムアウト処理

### キー入力のセキュリティ

- 信頼できるアプリケーションのみを対象にする設定 (オプション)
- ログへの機密情報記録を避ける

## トラブルシューティング

### デバイスが見つからない

**原因1: BluetoothアダプタがBLE Peripheral非対応**

BLE Peripheral Roleのサポート状況を確認:

```powershell
cd ResidentWin/BLETestConsole
dotnet run
```

出力で以下を確認:
```
IsPeripheralRoleSupported: True  ← これがFalseの場合は対応不可
```

**解決策:**
- BLE Peripheral対応のBluetoothアダプタを購入
- 推奨製品:
  - Intel AX200/AX201/AX210 搭載のWi-Fi+Bluetoothカード
  - TP-Link UB500 (BLE 5.0対応USBアダプタ)

**原因2: iOSアプリのバージョンが古い**

iOSアプリが最新版でない場合、Service UUIDベースのフィルタリングに対応していない可能性があります。

**解決策:**
- iOSアプリを最新版に更新してください
- または、Windows PCの名前を「EasyShortcutKey-Win」など「EasyShortcutKey」を含む名前に変更

PCの名前変更方法:
1. Windowsの設定を開く
2. システム → バージョン情報
3. 「このPCの名前を変更」をクリック
4. 「EasyShortcutKey-Win」など入力
5. 再起動

### 接続できない

- Bluetoothがオンになっているか確認
- 他のBLEデバイスとの競合がないか確認
- ResidentWin.exeを管理者権限で実行してみる

### キーボード入力が効かない

- 対象のアプリケーションがアクティブになっているか確認
- 管理者権限で実行されているアプリの場合、ResidentWinも管理者権限が必要
- 一部のゲームやセキュリティソフトはキーボードエミュレーションをブロックする場合があります

## 参考資料

### Windows BLE API
- [Windows.Devices.Bluetooth Namespace](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth)
- [GATT Server implementation](https://learn.microsoft.com/en-us/windows/uwp/devices-sensors/gatt-server)

### SendInput API
- [SendInput function (winuser.h)](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-sendinput)
- [Virtual-Key Codes](https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes)

### システムトレイアプリ
- [NotifyIcon Class](https://learn.microsoft.com/en-us/dotnet/api/system.windows.forms.notifyicon)

## ライセンス

(プロジェクト全体のライセンスに従う)

## 実装状況 (2025-10-05)

### ✅ 完了

- [x] Visual Studioプロジェクト作成 (.NET 9.0 WPF)
- [x] 基本的なトレイアプリ実装 (`TrayIconManager.cs`)
- [x] SendInput実装 (`KeyboardEmulator.cs`)
- [x] キーマッピング実装 (`KeyMapping.cs`)
- [x] BLE GATT Server実装 (`BLEManager.cs`)
- [x] モデルクラス実装 (`ShortcutCommand.cs`, `ConnectionState.cs`)
- [x] ユーティリティ実装 (`Logger.cs`, `ConfigManager.cs`)
- [x] アプリケーション統合 (`App.xaml.cs`)
- [x] ビルド成功確認

### 📝 実装済みファイル

```
ResidentWin/ResidentWin/
├── App.xaml / App.xaml.cs          ✅ 統合アプリケーション
├── app.manifest                     ✅ アプリケーションマニフェスト
├── ResidentWin.csproj              ✅ プロジェクトファイル
├── src/
│   ├── BLE/
│   │   └── BLEManager.cs           ✅ BLE GATT Server実装
│   ├── Keyboard/
│   │   ├── KeyboardEmulator.cs     ✅ SendInput API実装
│   │   └── KeyMapping.cs           ✅ キーマッピング
│   ├── Models/
│   │   ├── ShortcutCommand.cs      ✅ コマンドモデル
│   │   └── ConnectionState.cs      ✅ 接続状態
│   ├── UI/
│   │   └── TrayIconManager.cs      ✅ システムトレイ
│   └── Utils/
│       ├── Logger.cs                ✅ ログ管理
│       └── ConfigManager.cs         ✅ 設定管理
└── Resources/
    └── shortcuts.json               ✅ サンプルショートカット
```

### 🔧 TODO

- [ ] iPhoneアプリとの疎通確認
- [ ] 設定画面UI作成 (`SettingsWindow.xaml`)
- [ ] 自動起動設定機能 (`AutoStartup.cs`)
- [ ] ペアリング管理機能の強化
- [ ] エラーハンドリングの改善
- [ ] インストーラー作成
- [ ] アイコンファイル作成 (`.ico`)
- [ ] ユーザードキュメント作成

### 🐛 既知の問題

- DPI設定の警告 (WFO0003) - 機能には影響なし、後で修正可能
- BLE GATT Serverの実機テストが未実施

### 🚀 次のステップ

1. **実機テスト**: iPhoneアプリと接続してBLE通信をテスト
2. **キー入力テスト**: 各種アプリケーションでショートカットが正しく動作するか確認
3. **設定画面**: WPFで設定ウィンドウを作成
4. **自動起動**: Windows起動時の自動起動機能を実装
5. **リリース準備**: インストーラーとドキュメントの整備

---

**Last Updated**: 2025-10-05
**Version**: 0.1.0 (Initial Implementation)
