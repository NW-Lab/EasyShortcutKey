using System.Collections.Generic;
using System.Text.Json.Serialization;

namespace ResidentWin.Models
{
    /// <summary>
    /// iPhoneから受信するショートカットコマンドのモデル
    /// JSONデシリアライズに使用
    /// </summary>
    public class ShortcutCommand
    {
        /// <summary>
        /// ショートカットの一意識別子
        /// </summary>
        public string? Id { get; set; }

        /// <summary>
        /// ショートカット名
        /// </summary>
        public string? Name { get; set; }

        /// <summary>
        /// 押すキーのリスト (例: ["ctrl", "c"])
        /// </summary>
    // iOSクライアントは現状 `{ "keys": ["cmd","c"] }` のように小文字で送ってくるため
    // デフォルトの System.Text.Json (オプション未指定) では大文字小文字が区別されマッピングされない。
    // ここで JsonPropertyName("keys") を指定し受信できるようにする。
    [JsonPropertyName("keys")]
    public List<string>? Keys { get; set; }

        /// <summary>
        /// ショートカットの説明
        /// </summary>
        public string? Description { get; set; }

        /// <summary>
        /// カテゴリ (例: "Edit", "Navigation")
        /// </summary>
        public string? Category { get; set; }

        /// <summary>
        /// 対象アプリケーション (例: "Excel", "VSCode")
        /// </summary>
        public string? Application { get; set; }

        /// <summary>
        /// OSプラットフォーム (例: "Windows", "Mac")
        /// </summary>
        public string? Platform { get; set; }

        public override string ToString()
        {
            var keysStr = Keys != null ? string.Join("+", Keys) : "N/A";
            return $"{Name} ({keysStr})";
        }
    }
}
