using System;
using System.Drawing;
using System.Windows.Forms;
using ResidentWin.Models;
using ResidentWin.Utils;

namespace ResidentWin.UI
{
    /// <summary>
    /// システムトレイアイコンを管理するクラス
    /// </summary>
    public class TrayIconManager : IDisposable
    {
        private NotifyIcon? _notifyIcon;
        private ContextMenuStrip? _contextMenu;
        private ConnectionState _currentState = ConnectionState.Disconnected;

        public event EventHandler? StartBLERequested;
        public event EventHandler? StopBLERequested;
        public event EventHandler? SettingsRequested;
        public event EventHandler? ExitRequested;

        public TrayIconManager()
        {
            InitializeTrayIcon();
        }

        private void InitializeTrayIcon()
        {
            // コンテキストメニューを作成
            _contextMenu = new ContextMenuStrip();

            var startMenuItem = new ToolStripMenuItem("BLE接続開始", null, OnStartBLE);
            var stopMenuItem = new ToolStripMenuItem("BLE接続停止", null, OnStopBLE);
            var deviceNameMenuItem = new ToolStripMenuItem("BLEデバイス名を表示", null, OnShowDeviceName);
            var settingsMenuItem = new ToolStripMenuItem("設定", null, OnSettings);
            var aboutMenuItem = new ToolStripMenuItem("バージョン情報", null, OnAbout);
            var exitMenuItem = new ToolStripMenuItem("終了", null, OnExit);

            _contextMenu.Items.Add(startMenuItem);
            _contextMenu.Items.Add(stopMenuItem);
            _contextMenu.Items.Add(new ToolStripSeparator());
            _contextMenu.Items.Add(deviceNameMenuItem);
            _contextMenu.Items.Add(settingsMenuItem);
            _contextMenu.Items.Add(aboutMenuItem);
            _contextMenu.Items.Add(new ToolStripSeparator());
            _contextMenu.Items.Add(exitMenuItem);

            // NotifyIconを作成
            _notifyIcon = new NotifyIcon
            {
                Text = "ResidentWin - KeyboardGW",
                ContextMenuStrip = _contextMenu,
                Visible = true
            };

            // アイコンを設定 (デフォルトは灰色)
            UpdateIcon(ConnectionState.Disconnected);

            // ダブルクリックで設定を開く
            _notifyIcon.DoubleClick += (s, e) => SettingsRequested?.Invoke(this, EventArgs.Empty);

            Logger.Info("Tray icon initialized");
        }

        /// <summary>
        /// アイコンを接続状態に応じて更新
        /// </summary>
        public void UpdateIcon(ConnectionState state)
        {
            if (_notifyIcon == null) return;

            _currentState = state;

            // 状態に応じたアイコンの色を変更
            Color iconColor = state switch
            {
                ConnectionState.Disconnected => Color.Gray,
                ConnectionState.Waiting => Color.Blue,
                ConnectionState.Connected => Color.Green,
                ConnectionState.Pairing => Color.Yellow,
                ConnectionState.Error => Color.Red,
                _ => Color.Gray
            };

            // シンプルなアイコンを生成 (実際には.icoファイルを使用することを推奨)
            _notifyIcon.Icon = CreateSimpleIcon(iconColor);

            // ツールチップを更新
            _notifyIcon.Text = $"ResidentWin - {state}";

            Logger.Debug($"Tray icon updated: {state}");
        }

        /// <summary>
        /// 通知を表示
        /// </summary>
        public void ShowNotification(string title, string message, ToolTipIcon icon = ToolTipIcon.Info)
        {
            if (_notifyIcon == null) return;

            _notifyIcon.ShowBalloonTip(3000, title, message, icon);
            Logger.Debug($"Notification shown: {title} - {message}");
        }

        /// <summary>
        /// シンプルなアイコンを生成 (16x16のカラードット)
        /// </summary>
        private Icon CreateSimpleIcon(Color color)
        {
            const int size = 16;
            using var bitmap = new Bitmap(size, size);
            using (var g = Graphics.FromImage(bitmap))
            {
                g.Clear(Color.Transparent);

                // 中央に円を描画
                using var brush = new SolidBrush(color);
                g.FillEllipse(brush, 2, 2, size - 4, size - 4);

                // 外枠
                using var pen = new Pen(Color.Black, 1);
                g.DrawEllipse(pen, 2, 2, size - 4, size - 4);
            }

            IntPtr hIcon = bitmap.GetHicon();
            Icon icon = Icon.FromHandle(hIcon);
            return icon;
        }

        private void OnStartBLE(object? sender, EventArgs e)
        {
            Logger.Info("Start BLE requested from tray icon");
            StartBLERequested?.Invoke(this, EventArgs.Empty);
        }

        private void OnStopBLE(object? sender, EventArgs e)
        {
            Logger.Info("Stop BLE requested from tray icon");
            StopBLERequested?.Invoke(this, EventArgs.Empty);
        }

        private void OnShowDeviceName(object? sender, EventArgs e)
        {
            var deviceName = Environment.MachineName;
            MessageBox.Show(
                $"現在のBLEデバイス名（PC名）:\n\n{deviceName}\n\n" +
                "iPhoneアプリからこの名前で検索してください。\n" +
                "デバイス名を変更するには、Windowsの設定でPC名を変更してください。",
                "BLEデバイス名",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information
            );
            Logger.Info($"Displayed BLE device name: {deviceName}");
        }

        private void OnSettings(object? sender, EventArgs e)
        {
            Logger.Info("Settings requested from tray icon");
            SettingsRequested?.Invoke(this, EventArgs.Empty);
        }

        private void OnAbout(object? sender, EventArgs e)
        {
            MessageBox.Show(
                "ResidentWin - KeyboardGW for Windows\n\n" +
                "Version: 1.0.0\n" +
                "BLE Keyboard Emulator for Windows\n\n" +
                "© 2025 EasyShortcutKey Project",
                "バージョン情報",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information
            );
        }

        private void OnExit(object? sender, EventArgs e)
        {
            Logger.Info("Exit requested from tray icon");
            ExitRequested?.Invoke(this, EventArgs.Empty);
        }

        public void Dispose()
        {
            if (_notifyIcon != null)
            {
                _notifyIcon.Visible = false;
                _notifyIcon.Dispose();
                _notifyIcon = null;
            }

            _contextMenu?.Dispose();
            _contextMenu = null;

            Logger.Info("Tray icon disposed");
        }
    }
}
