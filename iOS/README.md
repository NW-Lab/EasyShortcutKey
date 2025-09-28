# EasyShortcutKey iOS

このフォルダには、iOS（SwiftUI）版の EasyShortcutKey アプリのソースとドキュメントが入ってるよ。まずは「Viewer（閲覧）」として動く最小構成、その後に BLE 接続＆デバイス連携を作り込む想定。

## システム構成（全体像）
iPhone —[BLE]→ KeyboardGW —[USB or BT Keyboard]→ PC（mac / Windows）
KeyboardGWはマイコンで作成。
### iPhone（このリポジトリの対象）
このフォルダで iOS アプリを開発するよ。現状は JSON からショートカットを読み込んで表示する Viewer モードを実装済み。

### BLE（計画）
使うプロファイルは検討中（GATT 想定）。Central 役として CoreBluetooth を使って、ペアリング済みのマイコンに接続する方針。

### マイコン（計画メモ）
- デバイスは未ESP32S3のM5StackのAtomS3を利用。
- ESPTOOLを使用して FW を簡単更新（将来的にはUF2を使用してドラッグ＆ドロップで Flash Memory 書き換え）　https://github.com/espressif/esptool/releases
- BLE 通信ができること
- USB Keyboard として動作（または Bluetooth Keyboard）。消費電力や安定性を考慮
- iPhone とペアリング固定（誰にでも接続されるのはセキュリティ的にNG）。相手機器 ID を保存
- LED/LCD などで接続状態が分かると嬉しい

補足: マイコン用のフォルダは、`/KeyboardGW` 

### USB/BT 側
USB でも Bluetooth Keyboard でも OK（両対応でも可）。最終的には PC 側でショートカット入力として認識されればOK。

---

## プロジェクトの開き方 / 実行方法（いまはこれがラク）
1. Xcode で `iOS/EasyShortcutKey.xcodeproj` を開く
2. ターゲット `EasyShortcutKey` を選択して実行（Simulator / 実機）
3. `Shortcuts.json` がターゲットに含まれていることを確認（Build Phases > Copy Bundle Resources）

補足: もし新規 Xcode プロジェクトを作ってファイルを移植したい場合は、`EasyShortcutKeyApp.swift` / `ContentView.swift` / `Models.swift` と `Shortcuts.json` を追加すれば動くよ。`Shortcuts.json` は必ずターゲットに含めてね。

---

## 含まれるファイル（iOS アプリ側）
- `Shortcuts.json` … 初回用のショートカット設定（バンドルに同梱）
- `EasyShortcutKeyApp.swift` … アプリエントリ
- `ContentView.swift` … シンプルな一覧 UI（タップでキー列をクリップボードにコピー）
- `Models.swift` … Codable モデルと JSON ローダ（Bundle から読み込み）
- （計画）`shortcutJsons/` … アプリごとのショートカット定義を入れるフォルダ
	- ここも追記: `config/shortcutJsons/` に置いたファイルを iOS アプリへ自動コピーする CI/ビルドスクリプトを後で用意する。今は `Shortcuts.json` 1本で読み込んでる。

JSON スキーマの詳細は `config/schema.json` を参照。`appName` / `groups[]` / `shortcuts[]` の階層で構成されるよ。

---

## アプリの動作（現状）
- マイコンと未接続時 … Viewer（一覧表示とコピーのみ）
- マイコンと接続時（予定） … タップしたショートカットをマイコン経由で PC に送出（仮想キーボード）

### ホーム画面（予定含む）
- ショートカットをシンプルに表示
- アプリ切り替えタブや、グループの展開/折りたたみ
- 設定画面へ移動するボタン
- マイコンの接続状態表示
- 表示/非表示の切り替え（通常は表示 or 未設定のものだけ）
- 非表示も含めて一時的に全部表示するトグル

### 設定画面（予定）
- マイコンとのペアリング管理
- `shortcutJsons` に登録されたアプリ一覧から、使うアプリを選択
- アプリ表示順の並び替え
- 選択アプリのショートカットを、ブラウザ版で使える `shortcuts.json` としてエクスポート（非表示の情報も含めた出力オプション）

---

## BLE を使うときの注意（実装済み）
Version 2.0で BLE 実装が完了しました。Info.plist に以下のキーを追加する必要があります：
- `NSBluetoothAlwaysUsageDescription` … Bluetooth 利用の目的説明

### Version 2.0 の新機能
- **KeyboardGWManager.swift**: BLE通信管理クラス
- **KeyboardGWPairingView.swift**: デバイスペアリング設定画面  
- **ContentView**: KeyboardGW接続状況表示とリアルタイムキー送信
- **SettingsView**: KeyboardGW設定項目追加

### KeyboardGWとの連携仕様
- **BLE Service UUID**: `12345678-1234-1234-1234-123456789ABC`
- **ショートカット特性**: `12345678-1234-1234-1234-123456789ABD`
- **ステータス特性**: `12345678-1234-1234-1234-123456789ABE`
- **ペアリング特性**: `12345678-1234-1234-1234-123456789ABF`

### 対応キー一覧
KeyboardGWは以下の特殊キーに完全対応：
- **修飾キー**: Shift, Ctrl/Control, Alt/Option, Cmd/Win/Command/Windows
- **矢印キー**: ↑↓←→ (記号) または Up/Down/Left/Right (英語)
- **特殊キー**: Space, Enter/Return, Tab, Esc/Escape, Backspace, Delete
- **ファンクションキー**: F1-F12
- **その他**: Home, End, PageUp, PageDown, Insert

### 動作モード
- **接続中**: ショートカットタップ → KeyboardGW経由でPCにリアルタイム送信 🎮
- **未接続**: ショートカットタップ → クリップボードにコピー（従来通り）📋

---

## よくある詰まりどころ（メモ）
- バンドル JSON が読めない → `Shortcuts.json` がターゲットに含まれてるか確認（Copy Bundle Resources）
- `ForEach` の表示がガタつく → `Models.swift` で `id` を stored property にして安定化済み
- Xcode が古い参照を掴んでる → 一度クリーンビルド or Xcode 再起動、Missing file が残ってないか確認

---

## ロードマップ（ざっくり）
- BLE 通信（GATT 定義・ペアリング・再接続）
- マイコン側プロトコルの仕様化（セキュアな相手制限）
- `shortcutJsons` の管理と iOS への自動反映（CI or ビルドスクリプト）
- 検索・フィルタ・詳細画面の追加
- UI/UX 改善（コピー後のトースト表示など）

---

## 変更履歴（ダイジェスト）
- 初期スキャフォールド追加（SwiftUI）
- `Shortcuts.json` ロードと一覧表示の実装
- `Item.swift`（テンプレート）削除、参照クリーンアップ済み
- **Version 2.0**: KeyboardGW連携機能を追加
  - `KeyboardGWManager.swift` … BLE通信でKeyboardGWと接続するマネージャークラス
  - `KeyboardGWPairingView.swift` … KeyboardGWのペアリング設定画面
  - `SettingsView.swift` … KeyboardGW設定項目を追加
  - `ContentView.swift` … 接続状況表示とショートカット送信機能を追加
  - ショートカットタップ時: KeyboardGW接続中は実際のキー送信、未接続時はクリップボードコピー

## Version 2.0 設定手順

### 1. Xcodeプロジェクト設定
1. Xcode で `iOS/EasyShortcutKey.xcodeproj` を開く
2. プロジェクトナビゲータで `EasyShortcutKey` プロジェクトを選択
3. `TARGETS` → `EasyShortcutKey` → `Info` タブを選択
4. `Custom iOS Target Properties` セクションで `+` ボタンをクリック
5. `Privacy - Bluetooth Always Usage Description` を追加
6. 値に「KeyboardGWデバイスと接続してショートカットキーを送信するために使用します。」と入力

### 2. KeyboardGWデバイス設定
KeyboardGWデバイス（ESP32S3 M5AtomS3）に以下のファームウェアを書き込んでください：
- PlatformIO を使って `KeyboardGW` フォルダ内のファームウェアをビルド・書き込み
- BLEデバイス名: `EasyShortcutKey-GW`
- サービスUUID: `12345678-1234-1234-1234-123456789ABC`

### 3. ペアリング手順
1. iOSアプリを起動
2. 右上の「設定」ボタンをタップ
3. 「KeyboardGW」→「デバイスを検索」をタップ
4. 発見されたデバイス一覧から対象デバイスを選択
5. 接続が完了すると、ホーム画面に「KeyboardGW接続中」と表示される

### 4. 使用方法
- **KeyboardGW接続中**: ショートカットをタップすると実際のキー入力がPCに送信される
- **KeyboardGW未接続**: ショートカットをタップするとクリップボードにコピーされる（従来通り）

