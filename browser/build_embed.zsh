#!/usr/bin/env zsh
# browser/build_embed.zsh
# Same as scripts/build_embed.zsh but placed inside browser/ for distribution convenience.

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

perl -0777 -e '
  use strict; use warnings;
  my $template_file = shift @ARGV;
  my $json_file = shift @ARGV;
  open my $t, "<:raw", $template_file or die "cannot open template: $template_file";
  local $/; my $tpl = <$t>;
  open my $j, "<:raw", $json_file or die "cannot open json: $json_file";
  my $json = <$j>;
  $tpl =~ s{(<script[^>]*id=["\']default-config["\'][^>]*type=["\']application/json["\'][^>]*>).*?(</script>)}{$1 . "\n" . $json . "\n" . $2}gsi;
  print $tpl;
' "$TEMPLATE" "$JSON" > "$OUT"

echo "生成しました: $OUT"
open "$OUT"
