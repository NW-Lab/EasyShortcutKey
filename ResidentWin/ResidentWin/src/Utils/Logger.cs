using System;
using System.IO;

namespace ResidentWin.Utils
{
    /// <summary>
    /// シンプルなログ出力クラス
    /// </summary>
    public static class Logger
    {
        private static readonly string LogDirectory = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
            "KeyboardGW",
            "Logs"
        );

        // 毎回同じログファイルを作り直す（起動時に内容をクリア）
        private static readonly string LogFilePath = Path.Combine(
            LogDirectory,
            "KeyboardGW.log"
        );

        public enum LogLevel
        {
            Debug,
            Info,
            Warning,
            Error
        }

        static Logger()
        {
            // ログディレクトリが存在しない場合は作成
            if (!Directory.Exists(LogDirectory))
            {
                Directory.CreateDirectory(LogDirectory);
            }

            // 既存ログを起動ごと初期化（容量肥大対策）
            try
            {
                if (File.Exists(LogFilePath))
                {
                    // 中身だけ空にする（ハンドル競合回避のため再作成ではなく truncate）
                    File.WriteAllText(LogFilePath, string.Empty);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to reset log file: {ex.Message}");
            }
        }

        public static void Debug(string message)
        {
            Log(LogLevel.Debug, message);
        }

        public static void Info(string message)
        {
            Log(LogLevel.Info, message);
        }

        public static void Warning(string message)
        {
            Log(LogLevel.Warning, message);
        }

        public static void Error(string message, Exception? ex = null)
        {
            var fullMessage = ex != null ? $"{message}\n{ex}" : message;
            Log(LogLevel.Error, fullMessage);
        }

        private static void Log(LogLevel level, string message)
        {
            var timestamp = DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff");
            var logMessage = $"[{timestamp}] [{level}] {message}";

            // コンソールに出力
            Console.WriteLine(logMessage);

            // ファイルに出力
            try
            {
                File.AppendAllText(LogFilePath, logMessage + Environment.NewLine);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"Failed to write to log file: {ex.Message}");
            }
        }

        public static void ClearOldLogs(int daysToKeep = 7)
        {
            try
            {
                var files = Directory.GetFiles(LogDirectory, "KeyboardGW_*.log");
                var cutoffDate = DateTime.Now.AddDays(-daysToKeep);

                foreach (var file in files)
                {
                    var fileInfo = new FileInfo(file);
                    if (fileInfo.CreationTime < cutoffDate)
                    {
                        File.Delete(file);
                        Info($"Deleted old log file: {fileInfo.Name}");
                    }
                }
            }
            catch (Exception ex)
            {
                Error("Failed to clear old logs", ex);
            }
        }
    }
}
