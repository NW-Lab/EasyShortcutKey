#!/usr/bin/env zsh
# build_embed.zsh
# Replace the content of <script id="default-config" type="application/json"> in browser/index.html
# with the contents of browser/shortcuts.json and write browser/index.built.html

set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${0}")/.." && pwd)
TEMPLATE="$ROOT_DIR/browser/index.html"
JSON="$ROOT_DIR/browser/shortcuts.json"
OUT="$ROOT_DIR/browser/index.built.html"

if [[ ! -f "$TEMPLATE" ]]; then
  echo "テンプレート $TEMPLATE が見つかりません。"
  exit 1
fi
if [[ ! -f "$JSON" ]]; then
  echo "JSON $JSON が見つかりません。"
  exit 1
fi

# Use perl to perform a robust multi-line replacement (keeps JSON raw)
perl -0777 -e '
  use strict; use warnings;
  my $template_file = shift @ARGV;
  my $json_file = shift @ARGV;
  open my $t, "<:raw", $template_file or die "cannot open template: $template_file";
  local $/; my $tpl = <$t>;
  open my $j, "<:raw", $json_file or die "cannot open json: $json_file";
  my $json = <$j>;
  $tpl =~ s{(<script[^>]*id=["'\''"]default-config["'\''"][^>]*type=["'\''"]application/json["'\''"][^>]*>).*?(</script>)}{$1 . "\n" . $json . "\n" . $2}gsi;
  print $tpl;
' "$TEMPLATE" "$JSON" > "$OUT"

echo "生成しました: $OUT"
open "$OUT"
