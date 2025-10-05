# BLE デバイス名の設定について

## 問題

Windows BLE GATT ServerのAPIでは、アドバタイズ時のデバイス名（Local Name）を直接設定することができません。代わりに、Windowsシステムの**PC名（コンピューター名）**がBLEデバイス名として使用されます。

## 対策方法

### 方法1: PC名を変更する（推奨）

iPhoneアプリから発見しやすくするため、PC名を変更します。

#### 手順

1. **設定を開く**
   - Windows 11: 設定 → システム → バージョン情報
   - Windows 10: 設定 → システム → 詳細情報

2. **「このPCの名前を変更」をクリック**

3. **新しい名前を入力**
   ```
   EasyShortcutKey-GW
   ```
   または
   ```
   KeyboardGW-Win
   ```

4. **再起動**

これにより、BLEアドバタイズ時のデバイス名が変更されます。

### 方法2: iPhoneアプリ側でService UUIDで検索する（✅ 既に実装済み）

PC名に依存せず、Service UUID (`12345678-1234-1234-1234-123456789ABC`) でデバイスを検索します。

#### iOSアプリの現在の実装

`KeyboardGWManager.swift`では既にService UUIDでフィルタリングしています:

```swift
// Service UUIDでフィルタリング（既存実装）
centralManager.scanForPeripherals(
    withServices: [CBUUID(string: "12345678-1234-1234-1234-123456789ABC")],
    options: [CBCentralManagerScanOptionAllowDuplicatesKey: false]
)
```

この実装により、PC名が何であっても、正しいService UUIDを持つResidentWinデバイスを発見できます。

この方法なら、PC名が何であっても、正しいサービスUUIDを持つデバイスを発見できます。

### 方法3: プログラムでPC名を取得して表示

ResidentWinアプリで現在のPC名（BLEデバイス名）を表示し、ユーザーに確認してもらいます。

#### 実装例

```csharp
using Windows.System.Profile;

public static string GetBluetoothDeviceName()
{
    // PCのホスト名 = BLEデバイス名
    return Environment.MachineName;
}
```

トレイアイコンのメニューに「BLEデバイス名を表示」を追加し、現在のPC名を表示します。

## 推奨アプローチ

**方法2（Service UUIDで検索）+ 方法3（PC名表示）の組み合わせ**が最も柔軟で使いやすいです。

1. iPhoneアプリはService UUIDで検索
2. 複数のデバイスが見つかった場合、ユーザーがPC名を確認して選択

## 技術的な背景

Windows 10/11のBLE APIは、主に**GATT Client**（ペリフェラルに接続する側）として設計されており、**GATT Server**（ペリフェラルとして動作する側）の機能は限定的です。

特に、`GattServiceProvider` APIでは:
- ✅ Service UUIDのアドバタイズは可能
- ✅ Characteristicの読み書きは可能
- ❌ デバイス名（Local Name）のカスタマイズは不可
- ❌ アドバタイズパケットの完全な制御は不可

そのため、PC名がそのままBLEデバイス名として使用されます。

## 参考資料

- [GattServiceProvider Class](https://learn.microsoft.com/en-us/uwp/api/windows.devices.bluetooth.genericattributeprofile.gattserviceprovider)
- [Bluetooth LE GATT Server](https://learn.microsoft.com/en-us/windows/uwp/devices-sensors/gatt-server)

---

**Last Updated**: 2025-10-05
