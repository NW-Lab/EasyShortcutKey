# Easy Shortcut Key

🔧 **様々なデバイスでショートカットキーを表示する統一システム**

ショートカットキーが覚えられない問題を解決するため、iOS、ブラウザ、M5Stack Tab5、M5Paper など複数のデバイスで共通の設定を使って、いつでもどこでもショートカットキーを確認できるシステムです。

## 🎯 特徴

- **統一された設定**: 1つのJSONファイルで全デバイスの設定を管理
- **マルチデバイス対応**: iOS、ブラウザ、M5Stack、M5Paper 対応予定
- **軽量設計**: ブラウザ版はHTMLとJS、設定用JSONファイルのみで動作
- **日本語対応**: 完全日本語UI・設定対応
-- **検索・フィルタ機能**: 現在は非表示。表示はJSONの `order` キーで制御します。

## 🚀 クイックスタート

### ブラウザ版の使用方法

1. **ファイルの配置**
   ```
  EasyShortcutKey/
   ├── browser/
   │   ├── index.html     # メインページ
   │   ├── app.js         # アプリケーションロジック
   │   └── style.css      # スタイル
   └── config/
       └── shortcuts.json # 設定ファイル
   ```

2. **ブラウザで開く**
  - `browser/index.html` をブラウザで直接開く（ダブルクリックでOK）
  - 「デフォルト設定を読み込み」ボタンをクリック、または
  - 独自のJSON設定ファイルをドラッグ&ドロップ

  ※ file://（ローカルでダブルクリック）で開いた場合、ブラウザによっては外部JSONの fetch が制限されることがあるため、`index.html` に埋め込まれたデフォルト設定がフォールバックとして使用されます（Windows/Macでの簡易利用を想定）。

#### 配布用 HTML を簡単に作る（ソース→ビルドワークフロー）

ブラウザ向けのソーステンプレートは `browser/index.source.html` に置いてある。テンプレートを編集してから、以下のコマンドで `browser/index.html` を再生成して配布用HTMLを作れる。

- macOS / Linux (zsh) : `browser/build_embed.zsh`
- Windows (PowerShell) : `browser/build_embed.ps1`

実行方法（macOS の例）:

```bash
cd browser
./build_embed.zsh        # デフォルトで生成後に既定ブラウザを開く
./build_embed.zsh --no-open   # 生成のみでブラウザは開かない
```

この操作は `index.source.html` をテンプレートとして `index.html` を上書きする（生成物は直接 `browser/index.html`）。配布前にテンプレートを編集してからビルドしてね。

#### 非技術ユーザ向け: デスクトップアプリでの起動 (推奨)

ブラウザの file:// 制限を回避し、PCに詳しくない人でもダブルクリックで起動できるように、Electron ラッパーを同梱している。

手順（配布元や管理者が一度実行すれば良い）:

1. Node.js をインストール（16+ 推奨）
2. リポジトリのルートで依存をインストール:

```bash
npm install
```

3. アプリを起動:

```bash
npm start
```

Electron 実行時は `shortcuts.json` を preload 経由で直接読み込むため、CORS の制限を気にせず動作する。配布用にネイティブ実行ファイルを作りたい場合は `electron-packager` 等でパッケージングできる。


3. **表示順について**
  - 表示は `config/shortcuts.json` 内の各 `program` エントリの `order` 値に従ってソートされます（小さい値から先に表示）。
  - 例: `"order": 1` が最優先で表示される。

## 📋 設定ファイル形式

### 基本構造

```json
[
  {
    "appName": "VS Code",
    "order": 1,
    "groups": [
      {
        "groupName": "GitHub Copilot",
        "order": 1,
        "shortcuts": [
          {
            "action": "承諾",
            "keys": ["Tab"],
            "description": "Copilotの提案を承諾"
          }
        ]
      }
    ]
  }
]
```

### 詳細な設定例

```json
[
  {
    "appName": "VS Code",           // アプリケーション名
    "order": 1,                     // 表示順序
    "icon": "icons/vscode.png",     // アイコンパス（オプション）
    "version": "1.80+",             // 対応バージョン（オプション）
    "groups": [
      {
        "groupName": "GitHub Copilot",           // 機能グループ名
        "order": 1,                              // グループ表示順序
        "description": "AI-powered suggestions", // グループ説明（オプション）
        "shortcuts": [
          {
            "action": "承諾",                      // アクション名（日本語可）
            "keys": ["Tab"],                      // キーの配列
            "description": "Copilotの提案を承諾", // 詳細説明
            "context": "エディタ内",               // 使用コンテキスト（オプション）
            "order": 1                              // グループ内での表示順（オプション）
          },
          {
            "action": "次の提案",
            "keys": ["Alt", "]"],                 // 修飾キー + 通常キー
            "description": "次の提案を表示"            // Program名でWindows/Linux向けの別エントリとして管理
          },
          {
            "action": "次の提案", 
            "keys": ["Option", "]"],              // Mac用の別設定
            "description": "次の提案を表示"         // Program名でMac向けの別エントリとして管理
          }
        ]
      }
    ]
  }
]
```

### 一時的に表示を消す（disEnable）

`disEnable` ブールを使うと、appName / group / shortcut のいずれのレベルでも一時的に表示をオフにできます。テンプレート編集時や公開前の調整で便利。

例:

```json
[
  {
    "appName": "VS Code",
    "order": 1,
    "disEnable": true, // この program は UI に表示されなくなる
    "groups": []
  },
  {
    "appName": "MyApp",
    "order": 2,
    "groups": [
      {
        "groupName": "Advanced",
        "order": 1,
        "disEnable": true, // この group は表示されない
        "shortcuts": []
      },
      {
        "groupName": "Common",
        "order": 2,
        "shortcuts": [
          {
            "action": "隠し機能",
            "keys": ["Ctrl","H"],
            "description": "普段は非表示",
            "disEnable": true // このショートカットのみ非表示
          }
        ]
      }
    ]
  }
]
```

## 🛠️ カスタマイズ

### 独自設定ファイルの作成

1. **簡単な設定から始める**
   - `config/examples/simple.json` を参考に作成
   - 最小限の項目: `program`, `order`, `groups`

2. **設定の検証**
   - `config/schema.json` でJSONスキーマ検証
   - ブラウザ版でリアルタイム検証

3. **OSごとの設定**
   ```json
   {
     "action": "コピー",
     "keys": ["Ctrl", "C"],
     "os": ["windows", "linux"]
   },
   {
     "action": "コピー", 
     "keys": ["Cmd", "C"],
     "os": ["mac"]
   }
   ```

## 📱 対応デバイス・プラットフォーム

### ✅ 現在利用可能
- **ブラウザ**: Chrome, Firefox, Safari, Edge
  - デスクトップ・モバイル対応
  - オフライン動作
  - 印刷機能

### 🚧 開発予定
- **iOS**: モバイルアプリ
- **M5Stack Tab5**: USBキーボード機能付きビューアー
- **M5Paper**: ePaperディスプレイ対応

## 📂 プロジェクト構造

```
EasyShortcutKey/
├── README.md              # このファイル
├── REQUIREMENTS.md        # 詳細要件定義
├── config/
│   ├── schema.json        # JSONスキーマ定義
│   ├── shortcuts.json     # メイン設定ファイル
│   └── examples/
│       └── simple.json    # シンプル設定例
├── browser/               # ブラウザ版実装
│   ├── index.html        # メインページ
│   ├── app.js            # アプリケーションロジック
│   └── style.css         # スタイルシート
├── ios/                  # iOS実装（予定）
├── m5stack-tab5/         # M5Stack Tab5実装（予定）
└── m5paper/              # M5Paper実装（予定）
```

## 🎨 UI・UX特徴

- **モダンなデザイン**: グラデーション背景、カード型レイアウト
- **レスポンシブ**: デスクトップ・モバイル対応
- **直感的操作**: ドラッグ&ドロップ、リアルタイム検索
- **アクセシビリティ**: キーボードナビゲーション、高コントラスト
- **印刷対応**: 紙での参照用に最適化された印刷レイアウト

## 🔧 開発・カスタマイズ

### ローカル開発

```bash
# リポジトリをクローン
git clone https://github.com/NW-Lab/EasyShortcutKey.git
cd EasyShortcutKey

# ブラウザでテスト
# browser/index.html をブラウザで開く

# ローカルサーバー（オプション）
python -m http.server 8000
# http://localhost:8000/browser/ でアクセス
```

### 設定ファイルの検証

```bash
# Python でJSON検証
python3 -c "
import json
with open('config/shortcuts.json') as f:
    data = json.load(f)
print('✅ Valid JSON format')
"
```

## � 配布用ビルド (Electron)

このリポジトリには簡易的な Electron ラッパーが含まれている。配布用のネイティブ実行ファイルを作るには、`electron-packager` などを使ってパッケージングするのが簡単。

簡単な手順（macOS / Windows 共通）:

```bash
# 1. 依存をインストール
npm install

# 2. (必要なら) 環境変数で NODE_ENV を設定しておく
# export NODE_ENV=production

# 3. パッケージ作成（例: macOS x64）
npm run package
```

`package.json` の `package` スクリプトは `electron-packager . EasyShortcutKey --platform=darwin --arch=x64 --out=dist --overwrite` を呼ぶ設定にしてある。Windows 用にする場合は `--platform=win32 --arch=ia32` や `--platform=win32 --arch=x64` を使う。

注意点:
- 生成されるバイナリは署名や notarize（macOS）などの追加手順が必要になる場合がある。配布先によってはウィルス対策ソフトにブロックされることがあるので、正式配布前に署名などを検討する。
- CI を使って macOS/Windows のビルドを自動化するのがおすすめ。


## �📄 ライセンス

MIT License - 自由に使用・改変・再配布可能

## 🤝 コントリビューション

プルリクエスト・Issue報告を歓迎します！

1. Fork this repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📞 サポート・お問い合わせ

- GitHub Issues: バグ報告・機能要望
- Documentation: `REQUIREMENTS.md` で詳細仕様を確認

---

**Easy Shortcut Key** - いつでもどこでも、ショートカットキーを忘れない 🔧✨
