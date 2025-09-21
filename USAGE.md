# 使用方法ガイド

## ブラウザ版 Easy Shortcut Key の使い方

### 基本的な使用方法

1. **アプリケーションの起動**
   - `browser/index.html` をブラウザで直接開く
   - または、ローカルサーバーを起動: `python3 -m http.server 8000`
   - ブラウザで `http://localhost:8000/browser/` にアクセス

2. **設定ファイルの読み込み**
   
   **方法1: デフォルト設定を使用**
   - 「デフォルト設定を読み込み」ボタンをクリック
   
   **方法2: 独自設定ファイルを使用**
  # 使用方法ガイド

  このドキュメントはブラウザ版 Easy Shortcut Key の使い方と、現在の設定フォーマットについての簡潔な案内です。古い自動OS判定や検索UIは廃止され、シンプルな配布と表示を優先した実装になっています。

  ## クイックスタート

  1. `browser/index.html` をダブルクリックしてブラウザで開く（最も簡単）
  2. または、ローカルサーバーを立てる場合:

  ```bash
  python3 -m http.server 8000
  # ブラウザで http://localhost:8000/browser/ にアクセス
  ```

  ## 設定ファイルの読み込み

  - 埋め込みデフォルト: `index.html` に埋め込んだ `default-config` を優先して読み込みます（file:// 環境での互換性のため）。
  - ローカルの `shortcuts.json` を fetch / XHR で読み込むフォールバックがありますが、ブラウザによっては file:// で制限される場合があります。
  - ドラッグ&ドロップ: JSON ファイルを画面にドラッグ&ドロップして読み込めます。

  ## 表示と順序

  - Program（アプリ）順および Group 内の順序は `order` プロパティで制御します。`order` がない場合は配列に記載した順序を維持します。

  ## 現在サポートしている JSON フィールド（主なもの）

  - `appName` (string) — 必須。表示されるプログラム名。
  - `order` (integer) — 任意。表示順序。
  - `icon` / `version` — 任意のメタ情報。
  - `disEnable` (boolean) — true の場合、その program/group/shortcut を表示しない（テンポラリな非表示に便利）。
  - `groups` — 配列。各グループは `groupName`, `order`, `description`, `shortcuts` を持ちます。
  - `shortcuts` の各項目:
    - `action` (string) — 表示名（例: "コピー"）
    - `keys` (array[string]) — 典型的なキー列（legacy）。
    - `steps` (array[object]) — マルチステップを表す配列。ブラウザUIでは現在キーのみを表示する（説明はJSONに保持）。各 step は `type`, `keys`, `description`, `os` などを持ち得る。
    - `os` (array[string]) — オプション。表示用のタグとして出るが自動判定/フィルタは行わない。

  参考: `config/schema.json` にスキーマ定義あり。

  ## ビルド（配布用 HTML の作成）

  テンプレートは `browser/index.source.html`。デフォルト設定を埋め込みたい場合、以下のスクリプトで `browser/index.html` を生成してください（`index.html` を上書きします）。

  - macOS / Linux (zsh): `browser/build_embed.zsh`
  - Windows (PowerShell): `browser/build_embed.ps1`

  例:

  ```bash
  cd browser
  zsh build_embed.zsh
  ```

  生成後の `index.html` は単一ファイルで配布できます。配布先でダブルクリックするだけで動く想定です。

  ## トラブルシューティング

  - ブラウザで古いJSを読み込んでるようなら強制リロード（Cmd/Ctrl+Shift+R）を行ってください。
  - `default-config` の埋め込みが期待どおりでない場合は `browser/index.source.html` を編集してから再度ビルドしてください。

  ## カスタマイズ

  - 見た目の調整は `browser/style.css` を編集
  - ロジックの変更は `browser/app.js` を編集

  ---

  詳細な設定例や運用ルール、他デバイスの情報は `config/` と `browser/README.md` を参照してください。
