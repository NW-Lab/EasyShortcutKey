using System;
using System.IO;
using System.Reflection;

namespace ResidentWin.Utils
{
    /// <summary>
    /// スタートアップフォルダのショートカットで自動起動制御
    /// </summary>
    public static class AutoStartupManager
    {
        private const string ShortcutFileName = "KeyboardGW.lnk";
        private static string StartupFolder => Environment.GetFolderPath(Environment.SpecialFolder.Startup);
        private static string ShortcutPath => Path.Combine(StartupFolder, ShortcutFileName);

        public static bool IsEnabled() => File.Exists(ShortcutPath);

        public static void Ensure(bool enabled)
        {
            if (enabled) CreateShortcut(); else RemoveShortcut();
        }

        public static void CreateShortcut()
        {
            try
            {
                var exePath = Assembly.GetExecutingAssembly().Location;
                var shellType = Type.GetTypeFromProgID("WScript.Shell");
                if (shellType == null) throw new InvalidOperationException("WScript.Shell not available");
                dynamic shell = Activator.CreateInstance(shellType)!;
                dynamic shortcut = shell.CreateShortcut(ShortcutPath);
                shortcut.TargetPath = exePath;
                shortcut.WorkingDirectory = Path.GetDirectoryName(exePath);
                shortcut.WindowStyle = 7; // Minimized
                shortcut.Description = "KeyboardGW (Windows版) 自動起動";
                try { shortcut.IconLocation = exePath; } catch { }
                shortcut.Save();
                Logger.Info($"Auto-start shortcut created: {ShortcutPath}");
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to create auto-start shortcut", ex);
            }
        }

        public static void RemoveShortcut()
        {
            try
            {
                if (File.Exists(ShortcutPath))
                {
                    File.Delete(ShortcutPath);
                    Logger.Info("Auto-start shortcut removed");
                }
            }
            catch (Exception ex)
            {
                Logger.Error("Failed to remove auto-start shortcut", ex);
            }
        }
    }
}
