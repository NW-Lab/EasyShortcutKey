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

## BLE を使うときの注意（先にメモ）
実装を入れるときは Info.plist に以下のキーが必要になるよ：
- `NSBluetoothAlwaysUsageDescription` … Bluetooth 利用の目的説明

Central 役でスキャン/接続する場合はこれで OK。バックグラウンド接続や Peripheral 役もやるなら、追加の設定が必要になることがある（そのときに追記）。

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

