using System;
using System.Collections.Generic;

namespace ResidentWin.Keyboard
{
    /// <summary>
    /// キー名をWindows仮想キーコードにマッピングする
    /// </summary>
    public static class KeyMapping
    {
        // 修飾キー
        public const ushort VK_LSHIFT = 0xA0;
        public const ushort VK_RSHIFT = 0xA1;
        public const ushort VK_LCONTROL = 0xA2;
        public const ushort VK_RCONTROL = 0xA3;
        public const ushort VK_LMENU = 0xA4;    // ALT
        public const ushort VK_RMENU = 0xA5;    // ALT
        public const ushort VK_LWIN = 0x5B;     // Windows キー
        public const ushort VK_RWIN = 0x5C;     // Windows キー

        // 特殊キー
        public const ushort VK_RETURN = 0x0D;   // Enter
        public const ushort VK_ESCAPE = 0x1B;   // Esc
        public const ushort VK_BACK = 0x08;     // Backspace
        public const ushort VK_TAB = 0x09;      // Tab
        public const ushort VK_SPACE = 0x20;    // Space
        public const ushort VK_DELETE = 0x2E;   // Delete
        public const ushort VK_HOME = 0x24;
        public const ushort VK_END = 0x23;
        public const ushort VK_PRIOR = 0x21;    // Page Up
        public const ushort VK_NEXT = 0x22;     // Page Down
        public const ushort VK_INSERT = 0x2D;

        // 矢印キー
        public const ushort VK_LEFT = 0x25;
        public const ushort VK_UP = 0x26;
        public const ushort VK_RIGHT = 0x27;
        public const ushort VK_DOWN = 0x28;

        // ファンクションキー
        public const ushort VK_F1 = 0x70;
        public const ushort VK_F2 = 0x71;
        public const ushort VK_F3 = 0x72;
        public const ushort VK_F4 = 0x73;
        public const ushort VK_F5 = 0x74;
        public const ushort VK_F6 = 0x75;
        public const ushort VK_F7 = 0x76;
        public const ushort VK_F8 = 0x77;
        public const ushort VK_F9 = 0x78;
        public const ushort VK_F10 = 0x79;
        public const ushort VK_F11 = 0x7A;
        public const ushort VK_F12 = 0x7B;

        /// <summary>
        /// キー名から仮想キーコードへのマッピング辞書
        /// </summary>
        private static readonly Dictionary<string, ushort> KeyMap = new(StringComparer.OrdinalIgnoreCase)
        {
            // 修飾キー
            { "ctrl", VK_LCONTROL },
            { "control", VK_LCONTROL },
            { "shift", VK_LSHIFT },
            { "alt", VK_LMENU },
            { "option", VK_LMENU },      // Mac互換
            { "win", VK_LWIN },
            { "cmd", VK_LWIN },          // Mac互換 (Cmdキー → Winキー)
            { "command", VK_LWIN },      // Mac互換
            { "meta", VK_LWIN },
            
            // Copilot キー (Win + C)
            { "copilot", VK_LWIN },      // 後でCキーと組み合わせる

            // 特殊キー
            { "enter", VK_RETURN },
            { "return", VK_RETURN },
            { "esc", VK_ESCAPE },
            { "escape", VK_ESCAPE },
            { "backspace", VK_BACK },
            { "tab", VK_TAB },
            { "space", VK_SPACE },
            { "delete", VK_DELETE },
            { "del", VK_DELETE },
            { "home", VK_HOME },
            { "end", VK_END },
            { "pageup", VK_PRIOR },
            { "pagedown", VK_NEXT },
            { "insert", VK_INSERT },

            // 矢印キー
            { "left", VK_LEFT },
            { "up", VK_UP },
            { "right", VK_RIGHT },
            { "down", VK_DOWN },
            { "arrowleft", VK_LEFT },
            { "arrowup", VK_UP },
            { "arrowright", VK_RIGHT },
            { "arrowdown", VK_DOWN },

            // ファンクションキー
            { "f1", VK_F1 },
            { "f2", VK_F2 },
            { "f3", VK_F3 },
            { "f4", VK_F4 },
            { "f5", VK_F5 },
            { "f6", VK_F6 },
            { "f7", VK_F7 },
            { "f8", VK_F8 },
            { "f9", VK_F9 },
            { "f10", VK_F10 },
            { "f11", VK_F11 },
            { "f12", VK_F12 },

            // 数字 (テンキーじゃない)
            { "0", 0x30 },
            { "1", 0x31 },
            { "2", 0x32 },
            { "3", 0x33 },
            { "4", 0x34 },
            { "5", 0x35 },
            { "6", 0x36 },
            { "7", 0x37 },
            { "8", 0x38 },
            { "9", 0x39 },

            // アルファベット
            { "a", 0x41 },
            { "b", 0x42 },
            { "c", 0x43 },
            { "d", 0x44 },
            { "e", 0x45 },
            { "f", 0x46 },
            { "g", 0x47 },
            { "h", 0x48 },
            { "i", 0x49 },
            { "j", 0x4A },
            { "k", 0x4B },
            { "l", 0x4C },
            { "m", 0x4D },
            { "n", 0x4E },
            { "o", 0x4F },
            { "p", 0x50 },
            { "q", 0x51 },
            { "r", 0x52 },
            { "s", 0x53 },
            { "t", 0x54 },
            { "u", 0x55 },
            { "v", 0x56 },
            { "w", 0x57 },
            { "x", 0x58 },
            { "y", 0x59 },
            { "z", 0x5A },

            // 記号
            { ";", 0xBA },      // VK_OEM_1 (;:)
            { "+", 0xBB },      // VK_OEM_PLUS
            { ",", 0xBC },      // VK_OEM_COMMA
            { "-", 0xBD },      // VK_OEM_MINUS
            { ".", 0xBE },      // VK_OEM_PERIOD
            { "/", 0xBF },      // VK_OEM_2 (/?)
            { "`", 0xC0 },      // VK_OEM_3 (`~)
            { "[", 0xDB },      // VK_OEM_4 ([{)
            { "\\", 0xDC },     // VK_OEM_5 (\|)
            { "]", 0xDD },      // VK_OEM_6 (]})
            { "'", 0xDE },      // VK_OEM_7 ('")
        };

        /// <summary>
        /// キー名を仮想キーコードに変換
        /// </summary>
        public static ushort GetVirtualKeyCode(string keyName)
        {
            if (KeyMap.TryGetValue(keyName, out var vkCode))
            {
                return vkCode;
            }

            // 見つからない場合は0を返す
            return 0;
        }

        /// <summary>
        /// 修飾キーかどうかを判定
        /// </summary>
        public static bool IsModifierKey(string keyName)
        {
            var normalized = keyName.ToLower();
            return normalized == "ctrl" || normalized == "control" ||
                   normalized == "shift" ||
                   normalized == "alt" || normalized == "option" ||
                   normalized == "win" || normalized == "cmd" || 
                   normalized == "command" || normalized == "meta";
        }

        /// <summary>
        /// Copilotキーなどの特殊キーを展開する
        /// 例: "copilot" → ["win", "c"]
        /// </summary>
        public static List<string> ExpandSpecialKey(string keyName)
        {
            var normalized = keyName.ToLower();
            
            if (normalized == "copilot")
            {
                return new List<string> { "win", "c" };
            }

            // その他の特殊キー展開が必要な場合はここに追加
            
            // 通常のキーはそのまま返す
            return new List<string> { keyName };
        }
    }
}
