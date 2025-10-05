# BLE GATT Server テスト用

このプログラムは、Windows BLE GATT Serverの動作確認用です。

## 実行方法

```powershell
cd ResidentWin/BLETestConsole
dotnet run
```

## 確認項目

1. Bluetoothアダプタの有無
2. Peripheral Roleのサポート状況
3. GATT Service Providerの作成
4. Characteristicの作成
5. Advertisementの開始

## トラブルシューティング

### "No Bluetooth adapter found"
- このPCにBluetoothアダプタがインストールされていません
- 外付けBluetoothアダプタを接続してください

### "Peripheral Role is NOT supported"
- このBluetoothアダプタはBLE Peripheral機能をサポートしていません
- **これが最も多い原因です**
- BLE 4.0以上かつPeripheral対応のアダプタが必要

### "Access Denied"
- アプリケーションに必要な権限がありません
- Bluetooth権限が必要です

### "RadioNotAvailable"
- Bluetoothがオフになっています
- Windowsの設定でBluetoothをオンにしてください

---

**重要**: Windows 10/11の多くのBluetoothアダプタは、BLE Peripheral (GATT Server) をサポートしていません。
Intel Wireless Bluetooth など、一部のチップセットのみがサポートしています。
