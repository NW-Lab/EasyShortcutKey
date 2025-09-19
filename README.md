# ShortcutKeyViewer

🔧 **様々なデバイスでショートカットキーを表示する統一システム**

ショートカットキーが覚えられない問題を解決するため、iOS、ブラウザ、M5Stack Tab5、M5Paper など複数のデバイスで共通の設定を使って、いつでもどこでもショートカットキーを確認できるシステムです。

## 🎯 特徴

- **統一された設定**: 1つのJSONファイルで全デバイスの設定を管理
- **マルチデバイス対応**: iOS、ブラウザ、M5Stack、M5Paper 対応予定
- **軽量設計**: ブラウザ版はHTMLとJS、設定用JSONファイルのみで動作
- **日本語対応**: 完全日本語UI・設定対応
- **OS自動判定**: 現在のOSに適したショートカットを自動表示
- **検索・フィルタ機能**: 必要なショートカットを素早く検索

## 🚀 クイックスタート

### ブラウザ版の使用方法

1. **ファイルの配置**
   ```
   ShortcutKeyViewer/
   ├── browser/
   │   ├── index.html     # メインページ
   │   ├── app.js         # アプリケーションロジック
   │   └── style.css      # スタイル
   └── config/
       └── shortcuts.json # 設定ファイル
   ```

2. **ブラウザで開く**
   - `browser/index.html` をブラウザで開く
   - 「デフォルト設定を読み込み」ボタンをクリック、または
   - 独自のJSON設定ファイルをドラッグ&ドロップ

3. **フィルタ・検索**
   - OS: Windows/Mac/Linux に自動適応
   - プログラム: 特定のアプリケーションでフィルタ
   - 検索: キーワードでショートカットを検索

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
            "description": "Copilotの提案を承諾",
            "os": ["windows", "mac", "linux"]
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
            "os": ["windows", "mac", "linux"],    // 対応OS
            "context": "エディタ内"               // 使用コンテキスト（オプション）
          },
          {
            "action": "次の提案",
            "keys": ["Alt", "]"],                 // 修飾キー + 通常キー
            "description": "次の提案を表示",
            "os": ["windows", "linux"]            // Windows/Linux用
          },
          {
            "action": "次の提案", 
            "keys": ["Option", "]"],              // Mac用の別設定
            "description": "次の提案を表示",
            "os": ["mac"]                         // Mac専用
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
     "keys": ["@CTRL", "C"],
     "os": ["windows", "linux"]
   },
   {
     "action": "コピー", 
     "keys": ["@CMD", "C"],
     "os": ["mac"]
   }
   ```

4. **拡張キーフォーマット**
   
   特殊キーと通常キーを区別し、複雑なキー操作を表現するための拡張フォーマットをサポート：

   ```json
   {
     "action": "特殊キー使用例",
     "keys": ["@CTRL", "@SHIFT", "P"],
     "description": "特殊キー（修飾キー）は@プレフィックス付き",
     "os": ["windows", "linux"]
   },
   {
     "action": "二段階操作の例",
     "keys": ["@CTRL+F, 検索語入力, @ENTER"],
     "description": "カンマ区切りで段階的な操作を表現",
     "os": ["windows", "linux"]
   },
   {
     "action": "エスケープシーケンス",
     "keys": ["@@", "@,"],
     "description": "リテラル@は@@、リテラル,は@,でエスケープ",
     "os": ["windows", "mac", "linux"]
   }
   ```

   **キーフォーマットルール:**
   - **特殊キー**: `@`プレフィックス付き（例: `@CTRL`, `@SHIFT`, `@ALT`）
   - **通常キー**: プレフィックスなし（例: `A`, `1`, `Space`）
   - **同時押し**: 同じ配列要素内で`+`区切り（例: `@CTRL+@SHIFT+P`）
   - **段階的操作**: カンマ区切り（例: `@CTRL+F, 検索語, @ENTER`）
   - **エスケープ**: `@@` → `@`、`@,` → `,`

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
ShortcutKeyViewer/
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
git clone https://github.com/NW-Lab/ShortcutKeyViewer.git
cd ShortcutKeyViewer

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

**ShortcutKeyViewer** - いつでもどこでも、ショートカットキーを忘れない 🔧✨
