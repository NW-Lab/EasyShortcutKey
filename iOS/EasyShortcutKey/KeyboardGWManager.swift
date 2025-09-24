import Foundation
import CoreBluetooth
import Combine

// KeyboardGWの接続状態を管理するクラス
class KeyboardGWManager: NSObject, ObservableObject {
    // MARK: - Published Properties
    @Published var isConnected: Bool = false
    @Published var isScanning: Bool = false
    @Published var discoveredDevices: [CBPeripheral] = []
    @Published var connectionStatus: String = "未接続"
    @Published var batteryLevel: Int = -1
    @Published var deviceName: String = ""
    
    // MARK: - Private Properties
    private var centralManager: CBCentralManager!
    private var connectedPeripheral: CBPeripheral?
    private var shortcutCharacteristic: CBCharacteristic?
    private var statusCharacteristic: CBCharacteristic?
    private var pairingCharacteristic: CBCharacteristic?
    
    // BLE UUIDs (KeyboardGWのConfig.hと一致させる)
    private let serviceUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABC")
    private let shortcutCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABD")
    private let statusCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABE")
    private let pairingCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABF")
    
    // MARK: - Initialization
    override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
    }
    
    // MARK: - Public Methods
    
    /// デバイスのスキャンを開始
    func startScanning() {
        startScanning(withServiceFilter: true)
    }
    
    /// デバイスのスキャンを開始（フィルター指定可能）
    func startScanning(withServiceFilter useFilter: Bool = true) {
        guard centralManager.state == .poweredOn else {
            connectionStatus = "Bluetoothが無効です"
            return
        }
        
        isScanning = true
        discoveredDevices.removeAll()
        connectionStatus = useFilter ? "デバイス検索中..." : "全デバイス検索中..."
        
        // デバッグ用: フィルターありなしを選択可能
        if useFilter {
            centralManager.scanForPeripherals(
                withServices: [serviceUUID],
                options: [CBCentralManagerScanOptionAllowDuplicatesKey: false]
            )
        } else {
            // フィルターなしスキャン（デバッグ用）
            centralManager.scanForPeripherals(
                withServices: nil,
                options: [CBCentralManagerScanOptionAllowDuplicatesKey: false]
            )
        }
        
        // 15秒後にスキャンを停止
        DispatchQueue.main.asyncAfter(deadline: .now() + 15) {
            self.stopScanning()
        }
    }
    
    /// デバイスのスキャンを停止
    func stopScanning() {
        centralManager.stopScan()
        isScanning = false
        if discoveredDevices.isEmpty && !isConnected {
            connectionStatus = "デバイスが見つかりません"
        }
    }
    
    /// 指定したデバイスに接続
    func connect(to peripheral: CBPeripheral) {
        stopScanning()
        connectionStatus = "接続中..."
        connectedPeripheral = peripheral
        centralManager.connect(peripheral, options: nil)
    }
    
    /// 現在のデバイスから切断
    func disconnect() {
        if let peripheral = connectedPeripheral {
            centralManager.cancelPeripheralConnection(peripheral)
        }
    }
    
    /// ショートカットコマンドを送信
    func sendShortcut(keys: [String]) {
        guard let characteristic = shortcutCharacteristic,
              let peripheral = connectedPeripheral,
              peripheral.state == .connected else {
            print("❌ ショートカット送信失敗：デバイスが接続されていません")
            return
        }
        
        let command = [
            "keys": keys,
            "keyCount": keys.count,
            "delay": 50
        ] as [String : Any]
        
        guard let jsonData = try? JSONSerialization.data(withJSONObject: command),
              let jsonString = String(data: jsonData, encoding: .utf8) else {
            print("❌ JSONデータの作成に失敗")
            return
        }
        
        let data = jsonString.data(using: .utf8)!
        peripheral.writeValue(data, for: characteristic, type: .withResponse)
        print("📤 ショートカット送信：\(keys.joined(separator: "+"))")
    }
    
    // MARK: - Private Methods
    
    private func setupCharacteristics(for peripheral: CBPeripheral) {
        guard let service = peripheral.services?.first(where: { $0.uuid == serviceUUID }) else {
            print("❌ サービスが見つかりません")
            return
        }
        
        for characteristic in service.characteristics ?? [] {
            switch characteristic.uuid {
            case shortcutCharUUID:
                shortcutCharacteristic = characteristic
                print("✅ ショートカット特性を設定")
                
            case statusCharUUID:
                statusCharacteristic = characteristic
                // ステータス通知を有効化
                peripheral.setNotifyValue(true, for: characteristic)
                print("✅ ステータス特性を設定（通知有効）")
                
            case pairingCharUUID:
                pairingCharacteristic = characteristic
                print("✅ ペアリング特性を設定")
                
            default:
                break
            }
        }
    }
}

// MARK: - CBCentralManagerDelegate
extension KeyboardGWManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        switch central.state {
        case .poweredOn:
            connectionStatus = "準備完了"
        case .poweredOff:
            connectionStatus = "Bluetoothがオフです"
        case .unauthorized:
            connectionStatus = "Bluetooth使用が許可されていません"
        case .unsupported:
            connectionStatus = "Bluetoothがサポートされていません"
        default:
            connectionStatus = "Bluetooth状態不明"
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        if !discoveredDevices.contains(where: { $0.identifier == peripheral.identifier }) {
            
            // デバッグ情報を詳しく出力
            print("🔍 デバイス発見：\(peripheral.name ?? "Unknown") (\(RSSI)dBm)")
            print("   UUID: \(peripheral.identifier)")
            print("   広告データ: \(advertisementData)")
            
            // KeyboardGW関連のデバイスかチェック
            let name = peripheral.name ?? ""
            let isKeyboardGW = name.contains("EasyShortcutKey") || name.contains("KeyboardGW") || name.contains("shortcut")
            
            if isKeyboardGW {
                print("   ✅ KeyboardGW関連デバイスを発見！")
            }
            
            // 全デバイス検索モードかどうかをチェック
            let isFullScanMode = connectionStatus.contains("全デバイス検索中")
            
            if isFullScanMode {
                // 全デバイススキャンモードの場合は全て追加
                discoveredDevices.append(peripheral)
            } else if isKeyboardGW {
                // 通常スキャンでもKeyboardGWは追加（名前ベースフィルタリング）
                discoveredDevices.append(peripheral)
                print("   📱 通常スキャンでKeyboardGWデバイスを追加")
            }
        }
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("✅ 接続成功：\(peripheral.name ?? "Unknown")")
        isConnected = true
        deviceName = peripheral.name ?? "KeyboardGW"
        connectionStatus = "接続済み"
        
        peripheral.delegate = self
        peripheral.discoverServices([serviceUUID])
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print("❌ 接続失敗：\(error?.localizedDescription ?? "Unknown error")")
        connectionStatus = "接続失敗"
        connectedPeripheral = nil
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("🔌 デバイス切断：\(peripheral.name ?? "Unknown")")
        isConnected = false
        connectionStatus = "未接続"
        deviceName = ""
        connectedPeripheral = nil
        shortcutCharacteristic = nil
        statusCharacteristic = nil
        pairingCharacteristic = nil
        
        if let error = error {
            print("❌ 切断エラー：\(error.localizedDescription)")
        }
    }
}

// MARK: - CBPeripheralDelegate
extension KeyboardGWManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        if let error = error {
            print("❌ サービス発見エラー：\(error.localizedDescription)")
            return
        }
        
        guard let services = peripheral.services else {
            print("❌ サービスがありません")
            return
        }
        
        for service in services {
            if service.uuid == serviceUUID {
                peripheral.discoverCharacteristics(
                    [shortcutCharUUID, statusCharUUID, pairingCharUUID],
                    for: service
                )
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        if let error = error {
            print("❌ 特性発見エラー：\(error.localizedDescription)")
            return
        }
        
        setupCharacteristics(for: peripheral)
    }
    
    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            print("❌ 値更新エラー：\(error.localizedDescription)")
            return
        }
        
        guard let data = characteristic.value else { return }
        
        if characteristic.uuid == statusCharUUID {
            // ステータス情報を受信
            if let jsonString = String(data: data, encoding: .utf8),
               let jsonData = jsonString.data(using: .utf8),
               let json = try? JSONSerialization.jsonObject(with: jsonData) as? [String: Any] {
                
                DispatchQueue.main.async {
                    if let battery = json["battery"] as? Int {
                        self.batteryLevel = battery
                    }
                    if let status = json["status"] as? String {
                        self.connectionStatus = status
                    }
                }
            }
        }
    }
    
    func peripheral(_ peripheral: CBPeripheral, didWriteValueFor characteristic: CBCharacteristic, error: Error?) {
        if let error = error {
            print("❌ 値書き込みエラー：\(error.localizedDescription)")
        } else {
            print("✅ 値書き込み成功")
        }
    }
}