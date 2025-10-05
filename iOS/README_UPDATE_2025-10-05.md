# iOSアプリ修正 - Service UUID ベースのデバイス検出

## 修正日
2025年10月5日

## 修正理由

Windows版ResidentWinがiOSアプリから検出されない問題に対応。

従来はデバイス名でフィルタリングしていたため、Windows PCの名前が「EasyShortcutKey」や「KeyboardGW」を含まない場合、検出されなかった。

## 修正内容

### 変更ファイル
- `iOS/EasyShortcutKey/KeyboardGWManager.swift`

### 変更概要

**変更前 (名前ベースのフィルタリング):**
```swift
let isKeyboardGW = name.contains("EasyShortcutKey") || name.contains("KeyboardGW") || name.contains("shortcut")

if isFullScanMode {
    self.discoveredDevices.append(peripheral)
} else if isKeyboardGW {
    self.discoveredDevices.append(peripheral)
}
```

**変更後 (Service UUIDベースのフィルタリング):**
```swift
// 広告データからService UUIDsを取得
let serviceUUIDs = advertisementData[CBAdvertisementDataServiceUUIDsKey] as? [CBUUID] ?? []

// 目的のService UUIDを含んでいるかチェック
let hasTargetServiceUUID = serviceUUIDs.contains(serviceUUID)

// Service UUIDが一致する、または名前で判定できる場合は追加
if hasTargetServiceUUID || isKeyboardGWByName {
    self.discoveredDevices.append(peripheral)
}
```

## 効果

1. **Windows版ResidentWinの検出が可能に**
   - Windows PCの名前に関わらず、Service UUIDが一致すれば検出される
   - デバイス名は「HP」や「DESKTOP-XXX」などでも問題なし

2. **後方互換性の維持**
   - 既存のAtomS3版KeyboardGWも引き続き動作
   - 名前ベースのチェックも残しているため、古いファームウェアにも対応

3. **より正確なフィルタリング**
   - Service UUIDが一致するデバイスのみを表示
   - 誤検出の可能性が減少

## 動作確認項目

### Windows版ResidentWin
- ✅ Service UUID `12345678-1234-1234-1234-123456789ABC` をアドバタイズに含める
- ✅ iOSアプリのスキャンで検出される
- ✅ 接続・ペアリングが成功する
- ✅ ショートカットキー送信が動作する

### AtomS3版KeyboardGW (既存)
- ✅ 引き続き検出される (Service UUID + デバイス名の両方でチェック)
- ✅ 既存の動作に影響なし

## 注意事項

### iOSアプリのアップデートが必要

この修正を適用したiOSアプリをApp Storeに公開する必要があります。

### 古いiOSアプリでの対応方法

アップデート前のiOSアプリを使用する場合、以下の回避策があります:

1. **Windows PCの名前を変更**
   - Windowsの設定 → システム → バージョン情報
   - 「このPCの名前を変更」
   - 「EasyShortcutKey-Win」など「EasyShortcutKey」を含む名前にする
   - 再起動

2. **iOSアプリをアップデート**
   - App Storeから最新版をインストール

## 技術詳細

### BLE Advertisement構造

```
Advertisement Packet
├── Device Name: "HP" (変更不可 - Windows PC名)
├── Service UUIDs: [12345678-1234-1234-1234-123456789ABC]  ← これで判定
└── その他の情報
```

### Core Bluetooth APIの挙動

```swift
// スキャン開始 (Service UUIDでフィルタリング)
centralManager.scanForPeripherals(
    withServices: [serviceUUID],  // このUUIDを含むデバイスのみが発見される
    options: [CBCentralManagerScanOptionAllowDuplicatesKey: false]
)

// デバイス発見時のコールバック
func centralManager(_ central: CBCentralManager, 
                    didDiscover peripheral: CBPeripheral, 
                    advertisementData: [String : Any], 
                    rssi RSSI: NSNumber) {
    
    // 広告データからService UUIDsを取得
    let serviceUUIDs = advertisementData[CBAdvertisementDataServiceUUIDsKey] as? [CBUUID]
    
    // Service UUIDの一致をチェック
    let hasTargetServiceUUID = serviceUUIDs?.contains(serviceUUID) ?? false
}
```

## リリース情報

### バージョン
- iOS: 次回リリース予定
- ResidentWin: 初回リリース

### 互換性
- iOS 14.0 以降
- Windows 10 (Build 19041) 以降

---

**注**: Macで確認する際は、Xcodeでビルド→実機転送して動作確認してください。
