# KeyboardGW (Windows版)

## 概要

Windows上で常駐して BLE GATT Server として動作し、iPhone の EasyShortcutKey アプリからショートカットキーを受信してキーボード入力をエミュレートするアプリケーションです。

AtomS3ハードウェア版のKeyboardGWと同じ機能をWindows PCで実現します。

## システム構成

```
iPhone —[BLE]→ KeyboardGW (Windows版 常駐アプリ) —[仮想キーボード入力]→ Windows アプリケーション
```

### ハードウェア版との違い

| 項目 | KeyboardGW (ハードウェア) | KeyboardGW (Windows版) |
|------|-----------|-------------|
| 実装環境 | ESP32-S3 (PlatformIO) | Windows (C#/.NET) |
| 通信 | USB HID + BLE | BLE |
| キー入力 | USB HIDキーボード | Win32 API (SendInput) |
| 電源 | USB給電 | PC電源 |
| 配置 | 外付けデバイス | 常駐ソフトウェア |

## 技術スタック

### 採用技術

- C# / .NET 9.0 (Windows専用ビルド)
- BLE Peripheral: Windows.Devices.Bluetooth (GattServiceProvider)
- キー入力: Win32 `SendInput`
- UI / 常駐: WPF + NotifyIcon (WinForms)
- ログ: 独自シンプルロガー (%APPDATA%\KeyboardGW\Logs)
- 配布: self‑contained 単一 EXE + ZIP

> 参考: 以前検討していた Python / C++ 代替案は現行スコープでは優先度低のため削除。

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

- ✅ システムトレイアイコン表示（接続状態色: Gray/Blue/Green/Yellow/Red）
- ✅ 右クリックメニュー（開始 / 停止 / デバイス名表示 / 設定(未実装) / 終了）
- ✅ トースト通知（起動・接続状態変更・エラー）
- ✅ Windows起動時の自動起動 (トレイメニューで切替可能)

### 4. 設定・管理機能

- ✅ ログ出力（起動ごとクリア / Debug 詳細切替は将来拡張）
- ✅ 設定ファイル（`%APPDATA%/KeyboardGW/config.json`）
- ⏳ ペアリング済みデバイス MAC ホワイトリスト
- ⏳ ショートカット設定外部 JSON 連携

## ファイル構成

```
KeyboardGW (Windows版) 物理フォルダ名はリポジトリ都合で `ResidentWin/` のままです（後方互換のため）：

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
.\KeyboardGW.exe   # (旧 ResidentWin.exe → AssemblyName 変更後)
```

### パッケージ (配布用 ZIP)

命名ポリシー: `KeyboardGW-Win-x64.zip`

PowerShell スクリプトを用意しています (初回はアイコン自動生成):

```powershell
cd ResidentWin
./build_package.ps1
```

出力: `ResidentWin/dist/KeyboardGW-Win-x64.zip`

手動でやる場合:

```powershell
dotnet publish ResidentWin/ResidentWin/ResidentWin.csproj `
   -c Release -r win-x64 --self-contained true `
   -p:PublishSingleFile=true -p:IncludeNativeLibrariesForSelfExtract=true `
   -o dist/publish
Compress-Archive -Path dist/publish/* -DestinationPath dist/KeyboardGW-Win-x64.zip -Force
```

## 使い方

1. **KeyboardGW.exe を起動**
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

## 自動起動 (Windowsログオン時)

アプリ内で対応済み。トレイアイコン右クリック → 「Windowsログオン時に自動起動を有効化 / 無効化」で切替。設定は `%APPDATA%/KeyboardGW/config.json` の `StartWithWindows` に保存され、スタートアップフォルダへ `KeyboardGW.lnk` を作成/削除します。

以下は手動で制御したい場合の代替手段です（通常は不要）。

### 方法1: スタートアップフォルダにショートカット (一番簡単 / 推奨)
1. エクスプローラのアドレスバーに `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup` と入力して開く
2. `KeyboardGW.exe` を右ドラッグ→「ショートカットをここに作成」
3. 完了 (次回ログオンから起動)

PowerShell で自動作成する例:
```powershell
$exe = "C:\\Path\\To\\KeyboardGW.exe"   # 実際の配置に書き換え
$startup = [Environment]::GetFolderPath('Startup')
$wsh = New-Object -ComObject WScript.Shell
$lnk = $wsh.CreateShortcut((Join-Path $startup 'KeyboardGW.lnk'))
$lnk.TargetPath = $exe
$lnk.WorkingDirectory = Split-Path $exe
$lnk.WindowStyle = 7   # Minimized
$lnk.Save()
```

### 方法2: レジストリ Run キー (シンプル / 非管理者領域)
ユーザーごと (HKCU) に登録:
```powershell
$exe = 'C:\\Path\\To\\KeyboardGW.exe'
New-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run' -Name 'KeyboardGW' -Value '"' + $exe + '"' -PropertyType String -Force
```
削除する場合:
```powershell
Remove-ItemProperty -Path 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Run' -Name 'KeyboardGW' -ErrorAction SilentlyContinue
```
注意: UAC 管理者権限での実行が必須な場所 (例: Program Files 配下) に置いた EXE を Run キーで起動すると「昇格されずに失敗」することがあるので、可能ならユーザー書き込み可能なフォルダに配置。

### 方法3: タスクスケジューラ (遅延起動 / 最小化 / 高権限が必要な場合)
1. 「タスク スケジューラ」を開く
2. 「タスクの作成」→ 名前: `KeyboardGW AutoStart`
3. トリガー: 「ログオン時」
4. 操作: プログラム開始 → `KeyboardGW.exe`
5. (任意) 全般タブで「最上の特権で実行する」チェック (管理者権限でショートカット送出が必要なケース)
6. 保存

PowerShell で登録する例 (標準権限):
```powershell
$exe = 'C:\\Path\\To\\KeyboardGW.exe'
$action = New-ScheduledTaskAction -Execute $exe
$trigger = New-ScheduledTaskTrigger -AtLogOn
Register-ScheduledTask -TaskName 'KeyboardGW AutoStart' -Action $action -Trigger $trigger -Description 'Auto start KeyboardGW at logon'
```
削除:
```powershell
Unregister-ScheduledTask -TaskName 'KeyboardGW AutoStart' -Confirm:$false
```

### どの方法を選ぶべき？
| 要件 | 推奨手段 |
|------|----------|
| とにかく簡単 | スタートアップフォルダ |
| レジストリで一元管理したい | Run キー |
| 管理者権限で起動 / 遅延起動したい | タスクスケジューラ |

### 実装方式メモ
内部実装: `AutoStartupManager` が `%APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup` に `KeyboardGW.lnk` を生成/削除。失敗時はログに記録されるので `KeyboardGW.log` を確認。

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
%APPDATA%\KeyboardGW\Logs\KeyboardGW.log
```

## セキュリティ考慮事項

### 管理者権限

一部のアプリケーション (管理者として実行されているもの) にキー入力を送るには、KeyboardGW (Windows版) 自体も管理者権限で実行する必要があります。

**対応方法**:
- アプリケーションマニフェストで `requireAdministrator` を設定
- または、ユーザーに右クリック → "管理者として実行" を案内

### BLEセキュリティ（今後強化予定）

- 現状: 平文 / オープン（UUID を知るクライアントから Write 可能）
- 予定: ペアリング済み MAC のみ受付 / 単純ハンドシェイク / アイドルタイムアウト

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
- KeyboardGW.exe を管理者権限で実行してみる

### キーボード入力が効かない

- 対象のアプリケーションがアクティブになっているか確認
- 管理者権限で実行されているアプリの場合、KeyboardGW (Windows版) も管理者権限が必要
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
- [x] ビルド/パブリッシュ/ZIP 生成
- [x] アプリアイコン (.ico) 生成スクリプト

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

- [ ] 設定画面 UI (`SettingsWindow.xaml`)
- [x] 自動起動設定 (スタートアップショートカット + トレイメニュー)
- [ ] BLE 簡易セキュリティ (MAC フィルタ / タイムアウト)
- [ ] 外部ショートカット設定のロード
- [ ] インストーラー (MSIX / WiX / winget manifest)
- [ ] ログレベル切替 & 詳細オプション

### 🐛 既知の問題

- DPI設定の警告 (WFO0003) - 機能には影響なし、後で修正可能
- なし（基本機能動作確認済み）

### 🚀 次のステップ

1. 設定画面の実装
2. （完了）自動起動機能実装済み → 追加で Registry / Task Scheduler を選べる拡張は任意
3. 簡易セキュリティ（MAC ホワイトリスト + タイムアウト）
4. 外部ショートカットファイルロード & ホットリロード
5. 配布: インストーラー + winget / Scoop 登録

---

**Last Updated**: 2025-10-07
**Version**: 0.1.0 (Initial Implementation / Renamed user-facing brand to KeyboardGW)
