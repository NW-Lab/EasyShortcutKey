using System.Collections.Generic;

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
