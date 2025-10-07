using System;
using System.Threading.Tasks;
using System.Reflection;
using System.Windows;
using System.Windows.Forms;
using ResidentWin.BLE;
using ResidentWin.Keyboard;
using ResidentWin.Models;
using ResidentWin.UI;
using ResidentWin.Utils;
using Application = System.Windows.Application;

namespace ResidentWin;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    private TrayIconManager? _trayIcon;
    private BLEManager? _bleManager;
    private KeyboardEmulator? _keyboardEmulator;
    private AppConfig? _config;
    private bool _autoStartupInitDone = false;

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        // ログ初期化
        Logger.Info("========================================");
    Logger.Info($"{Branding.AppDisplayName} Starting...");
        Logger.Info("========================================");

    // 実行中アセンブリ診断情報
    var asm = Assembly.GetExecutingAssembly();
    var ver = asm.GetName().Version?.ToString() ?? "(no version)";
    var loc = asm.Location;
    Logger.Info($"App Assembly Version: {ver}");
    Logger.Info($"Executable Path: {loc}");

        // 古いログをクリーンアップ (7日以上前)
        Logger.ClearOldLogs(7);

        // コマンドライン引数で --test が指定されていたらテストモード
        if (e.Args.Length > 0 && e.Args[0] == "--test")
        {
            Logger.Info("Running in TEST mode");
            Task.Run(async () => await ResidentWin.Test.BLETest.RunTest());
            return;
        }

        try
        {
            // 設定の読み込み
            _config = ConfigManager.Load();

            // キーボードエミュレータを初期化
            _keyboardEmulator = new KeyboardEmulator(_config.KeyInputDelayMs);
            Logger.Info("KeyboardEmulator initialized");

            // BLEマネージャーを初期化
            _bleManager = new BLEManager();
            _bleManager.ConnectionStateChanged += OnConnectionStateChanged;
            _bleManager.ShortcutReceived += OnShortcutReceived;
            Logger.Info("BLEManager initialized");

            // トレイアイコンを初期化
            _trayIcon = new TrayIconManager();
            _trayIcon.StartBLERequested += OnStartBLERequested;
            _trayIcon.StopBLERequested += OnStopBLERequested;
            _trayIcon.ExitRequested += OnExitRequested;
            _trayIcon.AutoStartupToggleRequested += OnAutoStartupToggleRequested;
            _trayIcon.NotificationToggleRequested += OnNotificationToggleRequested;
            Logger.Info("TrayIcon initialized");

            // 通知
            if (_config.ShowNotifications)
            {
                _trayIcon.ShowNotification(
                    Branding.AppDisplayName,
                    "KeyboardGW 起動",
                    ToolTipIcon.Info,
                    1000
                );
            }

            // 自動起動設定があればBLEを開始
            if (_config.AutoStartBLE)
            {
                Task.Run(async () => await StartBLEAsync());
            }

            // 自動起動設定同期 (StartWithWindows -> Startup ショートカット)
            AutoStartupManager.Ensure(_config.StartWithWindows);
            _trayIcon.UpdateAutoStartupMenu(_config.StartWithWindows);
            _trayIcon.UpdateNotificationMenu(_config.ShowNotifications);
            _autoStartupInitDone = true;

            Logger.Info("Application started successfully");
        }
        catch (Exception ex)
        {
            Logger.Error("Failed to start application", ex);
            System.Windows.MessageBox.Show(
                $"アプリケーションの起動に失敗しました:\n{ex.Message}",
                "エラー",
                MessageBoxButton.OK,
                MessageBoxImage.Error
            );
            Shutdown();
        }
    }

    protected override void OnExit(ExitEventArgs e)
    {
        Logger.Info("Application exiting...");

        // BLE停止
        if (_bleManager != null)
        {
            try
            {
                // 可能なら終了通知を飛ばしてから少し待機
                var t = _bleManager.SendShutdownNoticeAsync();
                t.Wait(300); // 短時間だけ同期待ち（UI凍結許容レベル）
            }
            catch { }
            _bleManager.Stop();
        }

        // トレイアイコン削除
        _trayIcon?.Dispose();

            Logger.Info("Application exited");
        base.OnExit(e);
    }

    private async void OnStartBLERequested(object? sender, EventArgs e)
    {
        await StartBLEAsync();
    }

    private void OnStopBLERequested(object? sender, EventArgs e)
    {
        _bleManager?.Stop();
        
        if (_config?.ShowNotifications == true)
            _trayIcon?.ShowNotification("BLE停止", "停止しました", ToolTipIcon.Info, 1000);
    }

    // 設定機能は削除済み

    private void OnExitRequested(object? sender, EventArgs e)
    {
        Logger.Info("Exit requested by user");
        Shutdown();
    }

    private void OnAutoStartupToggleRequested(object? sender, EventArgs e)
    {
        if (_config == null) return;
        var next = !_config.StartWithWindows;
        _config.StartWithWindows = next;
        ConfigManager.Save(_config);
        AutoStartupManager.Ensure(next);
        _trayIcon?.UpdateAutoStartupMenu(next);
        Logger.Info($"Auto-start state changed: {(next ? "Enabled" : "Disabled")}");
        if (_config.ShowNotifications && _autoStartupInitDone)
            _trayIcon?.ShowNotification("自動起動", next ? "有効" : "無効", ToolTipIcon.Info, 1000);
    }

    private void OnNotificationToggleRequested(object? sender, EventArgs e)
    {
        if (_config == null) return;
        _config.ShowNotifications = !_config.ShowNotifications;
        ConfigManager.Save(_config);
        _trayIcon?.UpdateNotificationMenu(_config.ShowNotifications);
        Logger.Info($"Notifications toggled: {(_config.ShowNotifications ? "ON" : "OFF")}");
        if (_config.ShowNotifications)
            _trayIcon?.ShowNotification("通知", "ON", ToolTipIcon.Info, 1000);
    }

    private void OnConnectionStateChanged(object? sender, ConnectionStateChangedEventArgs e)
    {
        // UIスレッドで実行
        Dispatcher.Invoke(() =>
        {
            _trayIcon?.UpdateIcon(e.State);

            // 接続状態変更のバルーン通知はユーザー要望で抑制
        });
    }

    private void OnShortcutReceived(object? sender, ShortcutCommand command)
    {
        Logger.Info($"Shortcut received: {command}");

        // キーボード入力を実行
        var success = _keyboardEmulator?.ExecuteShortcut(command) ?? false;

        if (!success)
        {
            Logger.Warning($"Failed to execute shortcut: {command}");
        }
    }

    private async Task StartBLEAsync()
    {
        if (_bleManager == null) return;

        if (_bleManager.IsRunning)
        {
            Logger.Info("BLE is already running");
            return;
        }

        Logger.Info("Starting BLE server...");
        
        var success = await _bleManager.StartAsync();

        if (success)
        {
            if (_config?.ShowNotifications == true)
                _trayIcon?.ShowNotification("BLE起動", "開始", ToolTipIcon.Info, 1000);
        }
        else
        {
            _trayIcon?.ShowNotification("BLEエラー", "開始失敗", ToolTipIcon.Error, 1000);
        }
    }
}

