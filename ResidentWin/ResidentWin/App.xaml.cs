using System;
using System.Threading.Tasks;
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

    protected override void OnStartup(StartupEventArgs e)
    {
        base.OnStartup(e);

        // ログ初期化
        Logger.Info("========================================");
        Logger.Info("ResidentWin Starting...");
        Logger.Info("========================================");

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
            _trayIcon.SettingsRequested += OnSettingsRequested;
            _trayIcon.ExitRequested += OnExitRequested;
            Logger.Info("TrayIcon initialized");

            // 通知
            if (_config.ShowNotifications)
            {
                _trayIcon.ShowNotification(
                    "ResidentWin",
                    "KeyboardGWが起動しました",
                    ToolTipIcon.Info
                );
            }

            // 自動起動設定があればBLEを開始
            if (_config.AutoStartBLE)
            {
                Task.Run(async () => await StartBLEAsync());
            }

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
        _bleManager?.Stop();

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
        {
            _trayIcon?.ShowNotification(
                "BLE停止",
                "BLE接続を停止しました",
                ToolTipIcon.Info
            );
        }
    }

    private void OnSettingsRequested(object? sender, EventArgs e)
    {
        // TODO: 設定画面を表示
        Logger.Info("Settings window requested (not implemented yet)");
        System.Windows.MessageBox.Show(
            "設定画面は未実装です",
            "設定",
            MessageBoxButton.OK,
            MessageBoxImage.Information
        );
    }

    private void OnExitRequested(object? sender, EventArgs e)
    {
        Logger.Info("Exit requested by user");
        Shutdown();
    }

    private void OnConnectionStateChanged(object? sender, ConnectionStateChangedEventArgs e)
    {
        // UIスレッドで実行
        Dispatcher.Invoke(() =>
        {
            _trayIcon?.UpdateIcon(e.State);

            if (_config?.ShowNotifications == true)
            {
                var icon = e.State switch
                {
                    ConnectionState.Connected => ToolTipIcon.Info,
                    ConnectionState.Error => ToolTipIcon.Error,
                    _ => ToolTipIcon.Info
                };

                _trayIcon?.ShowNotification(
                    "接続状態変更",
                    e.Message ?? e.State.ToString(),
                    icon
                );
            }
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
            {
                _trayIcon?.ShowNotification(
                    "BLE起動",
                    "BLE接続を開始しました。iPhoneから接続してください。",
                    ToolTipIcon.Info
                );
            }
        }
        else
        {
            _trayIcon?.ShowNotification(
                "エラー",
                "BLE接続の開始に失敗しました",
                ToolTipIcon.Error
            );
        }
    }
}

