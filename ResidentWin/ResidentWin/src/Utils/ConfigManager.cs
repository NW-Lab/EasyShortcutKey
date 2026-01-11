using System;
using System.IO;
using System.Text.Json;

namespace ResidentWin.Utils
{
    /// <summary>
    /// アプリケーション設定を管理するクラス
    /// </summary>
    public class AppConfig
    {
        /// <summary>
        /// 起動時に自動的にBLE接続を開始するか
        /// </summary>
        public bool AutoStartBLE { get; set; } = true;

        /// <summary>
        /// Windows起動時にアプリケーションを自動起動するか
        /// </summary>
        public bool StartWithWindows { get; set; } = false;

        /// <summary>
        /// デバッグモード
        /// </summary>
        public bool DebugMode { get; set; } = false;

        /// <summary>
        /// ペアリング済みデバイスのMACアドレス
        /// </summary>
        public string? PairedDeviceMac { get; set; }

        /// <summary>
        /// 最後に接続したデバイス名
        /// </summary>
        public string? LastConnectedDeviceName { get; set; }

        /// <summary>
        /// トースト通知を表示するか
        /// </summary>
    public bool ShowNotifications { get; set; } = false; // デフォルトOFF (デバッグ用にユーザーが有効化)

        /// <summary>
        /// キー入力の遅延 (ミリ秒)
        /// </summary>
        public int KeyInputDelayMs { get; set; } = 10;
    }

    public static class ConfigManager
    {
        private static readonly string ConfigDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "KeyboardGW"
        );

        private static readonly string ConfigFilePath = Path.Combine(
            ConfigDirectory,
            "config.json"
        );

        private static AppConfig? _config;

        /// <summary>
        /// 設定を読み込む
        /// </summary>
        public static AppConfig Load()
        {
            if (_config != null)
            {
                return _config;
            }

            try
            {
                if (!Directory.Exists(ConfigDirectory))
                {
                    Directory.CreateDirectory(ConfigDirectory);
                }

                if (File.Exists(ConfigFilePath))
                {
                    var json = File.ReadAllText(ConfigFilePath);
                    _config = JsonSerializer.Deserialize<AppConfig>(json) ?? new AppConfig();
                    Logger.Info("Configuration loaded successfully");
                }
                else
                {
                    _config = new AppConfig();
                    Save(_config);
                    Logger.Info("Created default configuration");
                }
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to load configuration", ex);
                _config = new AppConfig();
            }

            return _config;
        }

        /// <summary>
        /// 設定を保存する
        /// </summary>
        public static void Save(AppConfig config)
        {
            try
            {
                var options = new JsonSerializerOptions
                {
                    WriteIndented = true
                };

                var json = JsonSerializer.Serialize(config, options);
                File.WriteAllText(ConfigFilePath, json);
                _config = config;
                Logger.Info("Configuration saved successfully");
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to save configuration", ex);
            }
        }

        /// <summary>
        /// 現在の設定を取得する
        /// </summary>
        public static AppConfig GetCurrent()
        {
            return _config ?? Load();
        }
    }
}
