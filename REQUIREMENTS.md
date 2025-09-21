# Easy Shortcut Key 要件定義書

## 概要
ショートカットキーの参照・学習を支援するため、複数デバイスで共通のJSON設定を使ってショートカットを表示／実行できるシステムを提供する。

## 目的
- ショートカットの学習と記憶をサポートすること
- JSON一元管理で複数デバイスに同一設定を配布できること
- オフラインやローカル配布（単一HTMLファイル）で手軽に参照できること

## 対象デバイス

### iOS
- **用途**: モバイルでの参照・学習
- **要件**: iOSネイティブ／SwiftUIベースのビューア（既存の `iOS/` 実装）

### ブラウザ
- **用途**: デスクトップでの参照（作業中の補助）
- **要件**: 単一の静的HTML配布（`browser/index.html`）で動作すること。ファイル:// でも動作するよう、ビルド時に設定JSONを埋め込むワークフローを採用。

### M5Stack Tab5 / M5Paper（将来対応）
- **用途**: 物理デバイスでの参照・入力（Tab5はキーボードエミュレーション）

## JSON設定仕様（概要）
設定は配列で複数のアプリケーション（program）を定義する。重要なフィールドは現行実装に合わせて以下の通り。

```json
[
  {
    "appName": "VS Code",
    "order": 1,
    "disEnable": false,
    "groups": [
      {
        "groupName": "ナビゲーション",
        "order": 1,
        "shortcuts": [
          {
            "action": "コマンドパレットを開く",
            "keys": ["Cmd", "Shift", "P"],
            "description": "コマンドパレットを表示する",
            "os": ["mac"],
            "order": 1
          },
          {
            "action": "チャットを開く (複数ステップ)",
            "steps": [
              { "type": "keys", "keys": ["Cmd", "Shift", "P"], "description": "コマンドパレットを開く" },
              { "type": "input", "action": "Open Chat", "description": "'Open Chat' と入力して実行" }
            ],
            "description": "コマンドパレット経由でチャットを開く（表示用の多段手順）"
          }
        ]
      }
    ]
  }
]
```

### 主なフィールド説明（最新）

- `appName` (string, required)
  - アプリケーション名。旧称 `program` を置き換える。

- `disEnable` (boolean, optional)
  - true のときその要素（プログラム／グループ／ショートカット）は UI から一時的に隠される。

- `order` (integer, optional)
  - 明示的な表示順。未指定の場合は設定ファイル内の配列順を尊重する。

- `groups` (array, required)
  - グループ配列。各グループは `groupName` と `shortcuts` を持つ。

- `groupName` (string, required)
  - グループ名。

- `shortcuts` (array, required)
  - 各ショートカットは `action` と `description` を基本に、`keys` または `steps` のいずれかを含める。

- Shortcut 内の主要フィールド
  - `action` (string, required): ショートカットの名称／動作名
  - `description` (string, required): 説明（UI で必ず表示されるわけではないがメタ情報として保持）
  - `keys` (array of string, optional): 単一ステップのキー配列（修飾キー→メインキー）。既存の単純ショートカット向け。
  - `steps` (array of step objects, optional): 複雑な操作や手順を表現するための多段ステップ。`keys` を持たないショートカットを許容するために追加。
  - `os` (array of string, optional): 対応 OS（例: `"windows"`, `"mac"`, `"linux"`, `"ios"`, `"android"`）
  - `order` (integer, optional): グループ内での表示順

Step オブジェクト（`steps` 配列の要素、例）:

```json
{ "type": "keys", "keys": ["Cmd", "Shift", "P"], "description": "コマンドパレットを開く" }
```

主な `type` 値（現行実装で扱う）:
- `keys`: キー列を示す
- `input`: 文字列入力を表す（実行はしないが手順として表示）
- `ui`: UI操作（クリック等）の説明
- `wait`: 待機（duration を伴う）
- `macroRef`: 既存マクロの参照

## 表示／動作要件

- ブラウザ版は単一の `browser/index.html` を配布し、ファイルをダブルクリックして開けること（file:// 対応）。そのため、配布用ビルドは `browser/index.source.html` に canonical JSON を埋め込み `browser/index.html` を生成するワークフローを推奨する。
- 設定ファイルの検証では、`disEnable: true` の要素は UI に現れないこと。
- ショートカットは `keys` または `steps` のどちらかを持てる。`steps` がある場合、ブラウザUIでは簡潔にキー列のみを表示する（詳細説明は JSON に残す）。
- `order` が無い要素は配列順を尊重して表示する（安定ソート）。

## 機能要件ハイライト

1. 設定ファイル管理
   - JSON 設定の読み込みと検証（スキーマ準拠）
   - ブラウザ配布用の埋め込みビルド（`browser/build_embed.zsh` / `build_embed.ps1` による生成）

2. 表示機能
   - `appName` → `groups` → `shortcuts` の階層表示
   - OS フィルター、キーボード表記の正規化
   - 多段 `steps` の簡潔表示（キーだけ）

3. デバイス固有機能（要件コメント）
   - ブラウザ版: オフライン・file:// 対応、印刷、ドラッグ&ドロップでのアップロード
   - M5 系: 将来的にキーボードエミュレーションや SD 保存をサポート

## 技術仕様（簡易）

プロジェクト構成（抜粋）:

```
EasyShortcutKey/
├── config/
│   ├── shortcuts.json    # ソースとなる canonical 設定
  │   └── schema.json      # JSON スキーマ（設定検証用）
├── browser/
│   ├── index.source.html  # テンプレート（埋め込み用スクリプトブロックあり）
│   ├── index.html         # ビルド済み配布ファイル（埋め込み済み）
│   ├── app.js             # ブラウザ版ロジック
│   ├── style.css          # スタイル
│   └── build_embed.zsh    # 埋め込みビルド（zsh / macOS 用）
└── iOS/                   # iOS 実装（SwiftUI）

### 補足: `config/shortcutJsons/` の役割

リポジトリにはアプリ単位で分割された設定ファイルを置ける `config/shortcutJsons/` ディレクトリを用意している。

- 用途: 個別アプリやプラットフォームごとに設定を分割して管理するためのディレクトリ。チームで編集しやすく、必要に応じて個別ファイルをマージして `config/shortcuts.json` を生成できる。
- 命名と形式: 各ファイルは単一のアプリを表す JSON オブジェクト（`appName` を含む）とする。命名例: `VS Code (mac).json`, `Photoshop.json`。
- マージ時の簡易ルール:
  1. 各ファイルの `appName` を一意の識別子として扱う。
  2. プログラム・グループ・ショートカットそれぞれの `order` があればそれを優先する。未指定の場合はファイル内の配列順を保持する。
  3. 同一 `appName` が複数のファイルに存在する場合は、マージルールをプロジェクトの運用に合わせて決める（上書き/結合など）。

この仕組みは将来的に自動マージスクリプト（`scripts/` に配置）や CI チェックと組み合わせるのに便利。
```

## 開発優先度（現状）
1. JSON スキーマの安定化（`steps` / `disEnable` を含める）
2. ブラウザ版の安定配布（埋め込みビルド）
3. サンプル設定の整備
4. iOS・M5 系の順次対応

## 成功基準（更新）
- 統一された JSON により複数デバイスで設定が再現できること
- ブラウザ用の単一 HTML 配布で file:// からの起動が可能であること
- `disEnable` により一時的に非表示にできること
- `steps` を使って複雑な手順を表現できること
