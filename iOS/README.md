# EasyShortcutKey iOS Scaffold

このフォルダは簡易的な SwiftUI ベースの iOS アプリ雛形を含む。

含まれるファイル:
- `Shortcuts.json` - ブラウザ版のショートカットの一部をコピー
- `EasyShortcutKeyApp.swift` - App エントリ
- `ContentView.swift` - シンプルな一覧 UI
- `Models.swift` - Codable モデルと JSON ローダ

使い方:
1. Xcode で新規プロジェクトを作成 (App, Interface: SwiftUI, Language: Swift)
2. 生成されたプロジェクトにこのフォルダの Swift ファイルを追加する
3. `Shortcuts.json` をプロジェクトのターゲットにドラッグし、Copy items if needed にチェックして追加する
4. ビルドして実行

備考:
- これは最小限の雛形。今後、検索、フィルタ、永続化などを追加する予定。
