#!/usr/bin/env zsh
# browser/build_embed.zsh
# Simpler, robust implementation using awk/sed to avoid complex quoting.

set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${0}")" && pwd)
TEMPLATE="$ROOT_DIR/index.html"
JSON="$ROOT_DIR/shortcuts.json"
OUT="$ROOT_DIR/index.built.html"

if [[ ! -f "$TEMPLATE" ]]; then
  echo "テンプレート $TEMPLATE が見つかりません。"
  exit 1
fi
if [[ ! -f "$JSON" ]]; then
  echo "JSON $JSON が見つかりません。"
  exit 1
fi

# find start and end lines of the default-config script block
start=$(awk 'match($0, /<script[^>]*id=[^>]*default-config[^>]*>/){print NR; exit}' "$TEMPLATE") || true
if [[ -z "$start" ]]; then
  echo "default-config script block not found in $TEMPLATE"
  exit 1
fi
end=$(awk 'c && /<\/script>/{print NR; exit} /<script[^>]*id=[^>]*default-config[^>]*>/{c=1}' "$TEMPLATE") || true
if [[ -z "$end" ]]; then
  echo "closing </script> not found in $TEMPLATE"
  exit 1
fi

# assemble output: header, opening tag, raw JSON, closing tag, tail
sed -n "1,$((start-1))p" "$TEMPLATE" > "$OUT"
printf '<script id="default-config" type="application/json">\n' >> "$OUT"
cat "$JSON" >> "$OUT"
printf '\n</script>\n' >> "$OUT"
sed -n "$((end+1)),\$p" "$TEMPLATE" >> "$OUT"

echo "生成しました: $OUT"
open "$OUT"
