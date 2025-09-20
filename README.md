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

3. **表示順について**
  - 表示は `config/shortcuts.json` 内の各 `program` エントリの `order` 値に従ってソートされます（小さい値から先に表示）。
  - 例: `"order": 1` が最優先で表示される。

## 📋 設定ファイル形式

### 基本構造

```json
[
  {
    "program": "VS Code",
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
    "program": "VS Code",           // アプリケーション名
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

## 📄 ライセンス

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
