# Easy Shortcut Key

🔧 **様々なデバイスでショートカットキーを表示する統一システム**

ショートカットキーが覚えられない問題を解決するため、iOS、ブラウザ、M5Stack Tab5、M5Paper など複数のデバイスで共通の設定を使って、いつでもどこでもショートカットキーを確認できるシステムです。

## � Version 2.0 新機能

**🎮 リアルタイムキー送信機能**
- iOSアプリとKeyboardGWデバイス（M5Stack AtomS3）をBluetoothで連携
- ショートカットをタップするだけでPCに実際のキー操作を送信
- USB HIDキーボードとしてWindows/Mac両対応

**システム構成:**
```
iPhone (iOSアプリ) → [Bluetooth] → KeyboardGW → [USB] → PC (Windows/Mac)
```

**🔗 ペアリング方法:**
詳細なペアリング手順は [`Manual/Pairing.html`](Manual/Pairing.html) をご覧ください。

## �🎯 特徴

- **統一された設定**: 1つのJSONファイルで全デバイスの設定を管理
- **マルチデバイス対応**: iOS、ブラウザ、M5Stack、M5Paper 対応予定
- **軽量設計**: ブラウザ版はHTMLとJS、設定用JSONファイルのみで動作
- **日本語対応**: 完全日本語UI・設定対応
-- **検索・フィルタ機能**: 現在は非表示。表示はJSONの `order` キーで制御します。

# Easy Shortcut Key

🔧 様々なデバイスでショートカットキーを表示するための軽量ツール。

ショートカット設定は JSON で一元管理し、ブラウザや iOS、将来的には M5 系デバイスへ配布できることを目指しているよ。

## クイックスタート

- ブラウザ版: `browser/index.html` をダブルクリックして開くだけ（配布用は埋め込み済みの `index.html` を配布）
- 設定ファイルとスキーマ: `config/shortcuts.json`, `config/schema.json`
- ブラウザの詳細ドキュメントとビルド手順: `browser/README.md`

## 重要なフォルダ

- `config/` – canonical な設定とスキーマを置くディレクトリ。`config/shortcuts.json` がリポジトリ全体のメイン設定ファイル。
- `config/shortcutJsons/` – アプリ別の個別設定ファイルを置く場所。各ファイルは単一のアプリ（例: `VS Code (mac)`）を表す JSON オブジェクトで、後でまとめて `config/shortcuts.json` にマージできる。
   - 用途: チームや個別配布用にアプリごとに設定を分割・編集したいときに使う。
   - 命名例: `VS Code (mac).json`, `Photoshop.json`。
   - マージ時のルール: 各アプリ内で `order` が設定されていればそれを優先、未指定ならファイル内の配列順を保持する想定。
- `browser/` – ブラウザ用の静的アセット（`index.source.html`, `index.html`, `app.js`, `style.css`, ビルドスクリプトなど）。
- `iOS/` – iOS（SwiftUI）実装のソース。Version 2.0でKeyboardGWとのBluetooth連携機能を実装。
- `KeyboardGW/` – M5Stack AtomS3用のファームウェア（Arduino/PlatformIO）。BLE通信とUSB HID Keyboard機能を提供。
- `Manual/` – 使用方法とペアリング手順のドキュメント。WordPress等での公開用HTMLも含む。

## ドキュメント

- 使い方や設定の詳細は `USAGE.md` と `REQUIREMENTS.md` を参照してね。

---

License: MIT
