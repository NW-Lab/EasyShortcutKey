# scripts/ README

このディレクトリには開発／メンテ用のユーティリティスクリプトを置く。

## 運用ルール（重要）
- 新しいスクリプトを追加・更新したら、この `scripts/README.md` も必ず更新する。
- 各スクリプトには少なくとも以下を記載：
  - 名前・目的
  - 依存関係（例: Python 3）
  - 使い方（代表的なコマンド例）
  - 入出力（対象ディレクトリ／ファイル、上書き有無）
  - 終了コード（成功/失敗の規約）
  - 注意点（副作用、除外ファイルなど）

テンプレ:

```markdown
### <script-name>
- 目的: <what it does>
- 依存: <deps>
- 使い方:
  ```bash
  <command examples>
  ```
- 入出力: <inputs/outputs>
- 終了コード: <0=OK, 1/2=error など>
- 注意点: <notes>
```

---

## 現在のスクリプト

### assign_ids.py
- 目的: `config/shortcutJsons/` 配下の JSON に、program/group/shortcut 各レベルで欠けている `id` を UUID v4 で付与。
- 依存: Python 3（標準ライブラリのみ）
- 使い方:
  ```bash
  # 差分表示（dry-run）
  python3 scripts/assign_ids.py --dry-run

  # 実際に付与して保存
  python3 scripts/assign_ids.py --apply

  # 対象ディレクトリを指定
  python3 scripts/assign_ids.py --dir config/shortcutJsons --dry-run
  ```
- 入出力: デフォルトは `config/shortcutJsons/` 以下の *.json を対象。`--apply` 時はファイルを上書き保存。
- 注意点: バックアップ or Git 管理下での実行を推奨。

### validate_ids.py
- 目的: `id` の欠落・重複を検出してレポート（CIでの検証用）。
- 依存: Python 3（標準ライブラリのみ）
- 使い方:
  ```bash
  # ディレクトリ配下を検証
  python3 scripts/validate_ids.py --dir config/shortcutJsons

  # 単一ファイルを検証（例: 統合版）
  python3 scripts/validate_ids.py --file config/shortcuts.json

  # 両方指定も可
  python3 scripts/validate_ids.py --dir config/shortcutJsons --file config/shortcuts.json
  ```
- 終了コード: 0=OK / 1=検証失敗（欠落や重複）/ 2=入出力エラー
- 注意点: `*schema.json` は自動で除外。

### compare_ids.py
- 目的: 旧(base)と新(head)の JSON の `id` 集合を比較し、added/removed を一覧表示（「UUIDが最新に存在しない=廃止」の可視化）。
- 依存: Python 3（標準ライブラリのみ）
- 使い方:
  ```bash
  python3 scripts/compare_ids.py --old-file /tmp/base.json --new-file config/shortcuts.json
  ```
- 終了コード: 0=正常（情報表示のみ）/ 2=エラー
- 注意点: 旧/新どちらのファイルにも同一スキーマの JSON を渡すこと。

---

## CI での挙動

- Pull Request 時:
  - `validate_ids.py` で `id` 欠落・重複チェック（失敗したらPRが赤くなる）。
  - `assign_ids.py --dry-run` の結果をログ出力（付与予定の差分を確認）。
  - 可能なら `compare_ids.py` で base と head を比較し、removed/added を表示。

- main ブランチへの push 時:
  - `assign_ids.py --apply` で `id` 欠落を自動付与し、差分があれば bot で自動コミット。

---

## VS Code での実行

1) 統合ターミナルで直接実行

```bash
python3 scripts/assign_ids.py --dry-run
```

2) tasks.json を使ってワンクリック実行（例）

```json
{
  "version": "2.0.0",
  "tasks": [
    {
      "label": "assign-ids: dry-run",
      "type": "shell",
      "command": "python3",
      "args": ["scripts/assign_ids.py", "--dry-run"],
      "group": "none",
      "presentation": { "reveal": "always" }
    },
    {
      "label": "assign-ids: apply",
      "type": "shell",
      "command": "python3",
      "args": ["scripts/assign_ids.py", "--apply"],
      "group": "none",
      "presentation": { "reveal": "always" }
    }
  ]
}
```

3) 実行のヒント

- `--dry-run` で出力を確認してから `--apply` を実行するのが安全。
- macOS で Python が複数ある場合は `python3` のフルパス指定が確実。

