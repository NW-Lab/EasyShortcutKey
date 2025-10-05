using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading;
using ResidentWin.Models;
using ResidentWin.Utils;

namespace ResidentWin.Keyboard
{
    /// <summary>
    /// Windowsのキーボード入力をエミュレートするクラス
    /// SendInput APIを使用
    /// </summary>
    public class KeyboardEmulator
    {
        #region Win32 API Definitions

        [DllImport("user32.dll", SetLastError = true)]
        private static extern uint SendInput(uint nInputs, INPUT[] pInputs, int cbSize);

        [DllImport("user32.dll")]
        private static extern short GetKeyState(int nVirtKey);

        // INPUT 構造体のタイプ
        private const int INPUT_KEYBOARD = 1;

        // キーイベントフラグ
        private const uint KEYEVENTF_KEYDOWN = 0x0000;
        private const uint KEYEVENTF_KEYUP = 0x0002;
        private const uint KEYEVENTF_EXTENDEDKEY = 0x0001;

        [StructLayout(LayoutKind.Sequential)]
        private struct INPUT
        {
            public int type;
            public INPUTUNION u;
        }

        [StructLayout(LayoutKind.Explicit)]
        private struct INPUTUNION
        {
            [FieldOffset(0)]
            public MOUSEINPUT mi;
            [FieldOffset(0)]
            public KEYBDINPUT ki;
            [FieldOffset(0)]
            public HARDWAREINPUT hi;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct KEYBDINPUT
        {
            public ushort wVk;
            public ushort wScan;
            public uint dwFlags;
            public uint time;
            public IntPtr dwExtraInfo;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct MOUSEINPUT
        {
            public int dx;
            public int dy;
            public uint mouseData;
            public uint dwFlags;
            public uint time;
            public IntPtr dwExtraInfo;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct HARDWAREINPUT
        {
            public uint uMsg;
            public ushort wParamL;
            public ushort wParamH;
        }

        #endregion

        private readonly int _delayMs;

        public KeyboardEmulator(int delayMs = 10)
        {
            _delayMs = delayMs;
        }

        /// <summary>
        /// ショートカットコマンドを実行する
        /// </summary>
        public bool ExecuteShortcut(ShortcutCommand command)
        {
            if (command.Keys == null || command.Keys.Count == 0)
            {
                Logger.Warning($"Shortcut '{command.Name}' has no keys defined");
                return false;
            }

            try
            {
                Logger.Info($"Executing shortcut: {command}");

                // キーリストを展開 (Copilotキーなどの特殊キー対応)
                var expandedKeys = new List<string>();
                foreach (var key in command.Keys)
                {
                    expandedKeys.AddRange(KeyMapping.ExpandSpecialKey(key));
                }

                // 修飾キーと通常キーを分離
                var modifiers = expandedKeys
                    .Where(k => KeyMapping.IsModifierKey(k))
                    .Select(k => KeyMapping.GetVirtualKeyCode(k))
                    .Where(vk => vk != 0)
                    .ToList();

                var normalKeys = expandedKeys
                    .Where(k => !KeyMapping.IsModifierKey(k))
                    .Select(k => KeyMapping.GetVirtualKeyCode(k))
                    .Where(vk => vk != 0)
                    .ToList();

                if (modifiers.Count == 0 && normalKeys.Count == 0)
                {
                    Logger.Warning($"No valid keys found in shortcut: {command}");
                    return false;
                }

                // キー入力を実行
                PressKeys(modifiers, normalKeys);

                Logger.Info($"Shortcut executed successfully: {command.Name}");
                return true;
            }
            catch (Exception ex)
            {
                Logger.Error($"Failed to execute shortcut: {command.Name}", ex);
                return false;
            }
        }

        /// <summary>
        /// 修飾キーと通常キーを組み合わせて入力
        /// </summary>
        private void PressKeys(List<ushort> modifiers, List<ushort> normalKeys)
        {
            var inputs = new List<INPUT>();

            // 1. 修飾キーを押す (KeyDown)
            foreach (var modifier in modifiers)
            {
                inputs.Add(CreateKeyInput(modifier, KEYEVENTF_KEYDOWN));
            }

            // 2. 通常キーを押す (KeyDown)
            foreach (var key in normalKeys)
            {
                inputs.Add(CreateKeyInput(key, KEYEVENTF_KEYDOWN));
            }

            // 3. 通常キーを離す (KeyUp) - 逆順
            foreach (var key in normalKeys.AsEnumerable().Reverse())
            {
                inputs.Add(CreateKeyInput(key, KEYEVENTF_KEYUP));
            }

            // 4. 修飾キーを離す (KeyUp) - 逆順
            foreach (var modifier in modifiers.AsEnumerable().Reverse())
            {
                inputs.Add(CreateKeyInput(modifier, KEYEVENTF_KEYUP));
            }

            // SendInputで一括送信
            if (inputs.Count > 0)
            {
                uint result = SendInput((uint)inputs.Count, inputs.ToArray(), Marshal.SizeOf(typeof(INPUT)));
                
                if (result != inputs.Count)
                {
                    Logger.Warning($"SendInput failed: Expected {inputs.Count}, but {result} events were sent");
                }

                // キーボード処理の遅延
                if (_delayMs > 0)
                {
                    Thread.Sleep(_delayMs);
                }
            }
        }

        /// <summary>
        /// INPUT構造体を作成
        /// </summary>
        private INPUT CreateKeyInput(ushort virtualKeyCode, uint flags)
        {
            return new INPUT
            {
                type = INPUT_KEYBOARD,
                u = new INPUTUNION
                {
                    ki = new KEYBDINPUT
                    {
                        wVk = virtualKeyCode,
                        wScan = 0,
                        dwFlags = flags,
                        time = 0,
                        dwExtraInfo = IntPtr.Zero
                    }
                }
            };
        }

        /// <summary>
        /// キーが現在押されているかチェック
        /// </summary>
        public static bool IsKeyPressed(ushort virtualKeyCode)
        {
            return (GetKeyState(virtualKeyCode) & 0x8000) != 0;
        }
    }
}
