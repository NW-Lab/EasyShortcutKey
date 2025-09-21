# Browser (Web)版 — 詳細ドキュメント

このフォルダはブラウザ向けの配布物（静的 HTML / JS / CSS）をまとめた場所です。ここではブラウザ版の使い方、ビルド、配布に関する詳細をまとめています。

## すぐ使う（クイックスタート）

1. `EasyShortcutKey/browser/index.html` をダブルクリックしてブラウザで開くだけで動作します。
2. ローカルで別の設定を使いたいときは `browser/shortcuts.json` を編集するか、ページに JSON ファイルをドラッグ&ドロップしてください。

注意: ブラウザを `file://` で開くと `fetch` による外部 JSON 読み込みがブラウザ依存で制限される場合があります。本リポジトリでは埋め込み済みの `default-config` を利用することで、ダブルクリックで利用可能な配布物を作成できるようにしています。

---

## 配布用 HTML の生成 (ソース → ビルド)

ソーステンプレートは `browser/index.source.html` にあります。テンプレートを編集したら、以下のスクリプトで `browser/index.html` を生成します（生成物は `browser/index.html` に上書きされます）。

- macOS / Linux (zsh): `browser/build_embed.zsh`
- Windows (PowerShell): `browser/build_embed.ps1`

例（macOS）:

```bash
cd /path/to/EasyShortcutKey/browser
zsh build_embed.zsh        # 生成して既定ブラウザを開く
zsh build_embed.zsh --no-open   # 生成のみ
```

このビルドは `index.source.html` の `<script id="default-config" type="application/json">` ブロックに `browser/shortcuts.json` の内容を埋め込み、単一ファイルで配布できる `index.html` を作ります。

---

## ファイル配置（重要なファイル）

- `index.source.html` — テンプレート（編集はここ）
- `index.html` — 配布用（ビルドされたファイル、上書きされる）
- `shortcuts.json` — ブラウザ向けデフォルト設定（ビルド時に埋め込む）
- `app.js` / `style.css` — 実行コードとスタイル

---

## file:// と埋め込みの理由

多くのブラウザは `file://` からの `fetch` を厳しく制限します。手軽にダブルクリックで配布したい要件を満たすため、ビルド時に JSON を埋め込むワークフローを採用しました。これにより、ネットワークがなくても、単一の `index.html` をダブルクリックするだけで動作します。

---

## トラブルシューティング

- 生成後の `index.html` を編集すると次回のビルドで上書きされます。テンプレートは `index.source.html` を変更してください。
- ブラウザで表示が更新されない場合は強制リロード（Cmd/Ctrl+Shift+R）を試してください。

---

詳細な設定形式や JSON のスキーマ、iOS や他デバイス向けの情報はルートの `config/schema.json` と `config/shortcuts.json` を参照してください。
