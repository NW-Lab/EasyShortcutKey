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
        private ToolStripMenuItem? _autoStartupMenuItem;
        private ToolStripMenuItem? _notificationToggleMenuItem;

        public event EventHandler? StartBLERequested;
        public event EventHandler? StopBLERequested;
    // 設定ダイアログは削除済み
        public event EventHandler? ExitRequested;
        public event EventHandler? AutoStartupToggleRequested;
        public event EventHandler? NotificationToggleRequested;

        public TrayIconManager()
        {
            InitializeTrayIcon();
        }

        private void InitializeTrayIcon()
        {
            _contextMenu = new ContextMenuStrip();

            var startMenuItem = new ToolStripMenuItem("BLE接続開始", null, OnStartBLE);
            var stopMenuItem = new ToolStripMenuItem("BLE接続停止", null, OnStopBLE);
            _autoStartupMenuItem = new ToolStripMenuItem("Windowsログオン時に自動起動を有効化", null, OnAutoStartupToggle);
            var deviceNameMenuItem = new ToolStripMenuItem("BLEデバイス名を表示", null, OnShowDeviceName);
            _notificationToggleMenuItem = new ToolStripMenuItem("通知を有効化", null, OnNotificationToggle);
            // 設定メニュー削除
            var aboutMenuItem = new ToolStripMenuItem("バージョン情報", null, OnAbout);
            var exitMenuItem = new ToolStripMenuItem("終了", null, OnExit);

            _contextMenu.Items.Add(startMenuItem);
            _contextMenu.Items.Add(stopMenuItem);
            _contextMenu.Items.Add(new ToolStripSeparator());
            _contextMenu.Items.Add(_autoStartupMenuItem);
            _contextMenu.Items.Add(new ToolStripSeparator());
            _contextMenu.Items.Add(deviceNameMenuItem);
            _contextMenu.Items.Add(_notificationToggleMenuItem);
            // (設定メニューなし)
            _contextMenu.Items.Add(aboutMenuItem);
            _contextMenu.Items.Add(new ToolStripSeparator());
            _contextMenu.Items.Add(exitMenuItem);

            _notifyIcon = new NotifyIcon
            {
                Text = $"{Branding.AppDisplayName}",
                ContextMenuStrip = _contextMenu,
                Visible = true
            };

            UpdateIcon(ConnectionState.Disconnected);
            // ダブルクリック動作無し（設定機能削除）
            Logger.Info("Tray icon initialized");
        }

        public void UpdateIcon(ConnectionState state)
        {
            if (_notifyIcon == null) return;
            _currentState = state;
            Color iconColor = state switch
            {
                ConnectionState.Disconnected => Color.Gray,
                ConnectionState.Waiting => Color.Blue,
                ConnectionState.Connected => Color.Green,
                ConnectionState.Pairing => Color.Yellow,
                ConnectionState.Error => Color.Red,
                _ => Color.Gray
            };
            _notifyIcon.Icon = CreateSimpleIcon(iconColor);
            _notifyIcon.Text = $"{Branding.AppDisplayName} - {state}";
            Logger.Debug($"Tray icon updated: {state}");
        }

        public void ShowNotification(string title, string message, ToolTipIcon icon = ToolTipIcon.Info, int timeoutMs = 1000)
        {
            if (_notifyIcon == null) return;
            if (timeoutMs < 100) timeoutMs = 100;
            _notifyIcon.ShowBalloonTip(timeoutMs, title, message, icon);
            Logger.Debug($"Notification shown ({timeoutMs}ms): {title} - {message}");
        }

        public void UpdateAutoStartupMenu(bool enabled)
        {
            if (_autoStartupMenuItem == null) return;
            _autoStartupMenuItem.Text = enabled ? "Windowsログオン時の自動起動を無効化" : "Windowsログオン時に自動起動を有効化";
        }

        public void UpdateNotificationMenu(bool enabled)
        {
            if (_notificationToggleMenuItem == null) return;
            _notificationToggleMenuItem.Text = enabled ? "通知を無効化" : "通知を有効化";
        }

        private Icon CreateSimpleIcon(Color color)
        {
            const int size = 16;
            using var bitmap = new Bitmap(size, size);
            using (var g = Graphics.FromImage(bitmap))
            {
                g.Clear(Color.Transparent);
                using var brush = new SolidBrush(color);
                g.FillEllipse(brush, 2, 2, size - 4, size - 4);
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
                $"現在のBLEデバイス名（PC名）：\n\n{deviceName}\n\n" +
                "iPhoneアプリからこの名前で検索してください。\n" +
                "デバイス名を変更するには Windows の設定で PC 名を変更してください。",
                "BLEデバイス名",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information);
            Logger.Info($"Displayed BLE device name: {deviceName}");
        }

        // 設定機能削除のためハンドラなし

        private void OnAbout(object? sender, EventArgs e)
        {
            MessageBox.Show(
                $"{Branding.AppDisplayName} for Windows\n\nVersion: 1.0.0\nBLE Keyboard Emulator for Windows\n\n© 2025 EasyShortcutKey Project",
                "バージョン情報",
                MessageBoxButtons.OK,
                MessageBoxIcon.Information);
        }

        private void OnExit(object? sender, EventArgs e)
        {
            Logger.Info("Exit requested from tray icon");
            ExitRequested?.Invoke(this, EventArgs.Empty);
        }

        private void OnAutoStartupToggle(object? sender, EventArgs e)
        {
            AutoStartupToggleRequested?.Invoke(this, EventArgs.Empty);
        }

        private void OnNotificationToggle(object? sender, EventArgs e)
        {
            NotificationToggleRequested?.Invoke(this, EventArgs.Empty);
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
