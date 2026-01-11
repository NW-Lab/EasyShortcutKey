# KeyboardGW (Windows版) BLE テストコンソール

KeyboardGW (Windows版) が動作する環境で Bluetooth アダプタが BLE Peripheral (GATT Server) を正しくサポートしているか、最小限の API で検証するためのコンソールツールです。問題があった場合はまずここで原因切り分け。

## 実行方法

```powershell
cd ResidentWin/BLETestConsole
dotnet run
```

## 確認項目

1. Bluetooth アダプタ存在確認
2. Peripheral Role サポート可否
3. GattServiceProvider 作成結果
4. Characteristic 生成結果
5. Advertisement 開始結果 (エラー/Success)

## トラブルシューティング

### "No Bluetooth adapter found"
- このPCにBluetoothアダプタがインストールされていません
- 外付けBluetoothアダプタを接続してください

### "Peripheral Role is NOT supported"
- Bluetooth アダプタが GATT Server をサポートしていません
- **最も多い失敗要因**
- 対応デバイス例: Intel AX200/AX201/AX210, 一部 Realtek チップ

### "Access Denied"
- Bluetooth スタック初期化に失敗 / OS 側拒否
- Windows 再起動 or 他のアプリが BLE を占有していないか確認

### "RadioNotAvailable"
- Bluetoothがオフになっています
- Windowsの設定でBluetoothをオンにしてください

---

---

**重要**: 一般的な低価格 USB ドングルは Peripheral Role を持たないことが多いです。`IsPeripheralRoleSupported: False` の場合はハード的制約なのでアダプタ交換を検討してください。
