Migration performed: created backups and exported authoritative config copies for verification.

Backups created (do not delete until you confirm everything is good):
- config_VSCode_win_en.json.bak
- config_Excel_mac_en.json.bak
- ios_VSCode_win_en.json.bak
- ios_Excel_mac_en.json.bak

Authoritative copies exported to iOS folder with suffix .from_config.json so you can diff/replace if desired.

Next steps:
1) Review files in backups/shortcut_migration_2025-10-02/
2) If OK, replace iOS files with the .from_config.json versions (or rename them)
3) Optionally remove the old misnamed folder if still present and commit changes
