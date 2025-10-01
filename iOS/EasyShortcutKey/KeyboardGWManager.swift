import Foundation
import CoreBluetooth
import Combine

// KeyboardGWの接続状態を管理するクラス
class KeyboardGWManager: NSObject, ObservableObject {
    // シングルトンインスタンス
    static let shared = KeyboardGWManager()
    
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
    private var savedPeripheralIdentifier: UUID?
    private var isAutoReconnecting: Bool = false
    
    // BLE UUIDs (KeyboardGWのConfig.hと一致させる)
    private let serviceUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABC")
    private let shortcutCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABD")
    private let statusCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABE")
    private let pairingCharUUID = CBUUID(string: "12345678-1234-1234-1234-123456789ABF")
    
    // MARK: - Initialization
    private override init() {
        super.init()
        centralManager = CBCentralManager(delegate: self, queue: nil)
        loadSavedPeripheral()
    }
    
    // MARK: - Auto-reconnection methods
    
    /// 保存されたデバイス情報を読み込み
    private func loadSavedPeripheral() {
        if let savedUUIDString = UserDefaults.standard.string(forKey: "SavedPeripheralUUID"),
           let savedUUID = UUID(uuidString: savedUUIDString) {
            savedPeripheralIdentifier = savedUUID
            print("💾 保存されたデバイス情報を読み込み: \(savedUUIDString)")
        }
    }
    
    /// デバイス情報を保存
    private func savePeripheral(_ peripheral: CBPeripheral) {
        savedPeripheralIdentifier = peripheral.identifier
        UserDefaults.standard.set(peripheral.identifier.uuidString, forKey: "SavedPeripheralUUID")
        UserDefaults.standard.set(peripheral.name, forKey: "SavedPeripheralName")
        print("💾 デバイス情報を保存: \(peripheral.name ?? "Unknown") (\(peripheral.identifier))")
    }
    
    /// 保存されたデバイスに自動再接続を試行
    private func attemptAutoReconnection() {
        guard let savedUUID = savedPeripheralIdentifier,
              centralManager.state == .poweredOn,
              !isConnected && !isAutoReconnecting else {
            return
        }
        
        isAutoReconnecting = true
        
        DispatchQueue.main.async {
            self.connectionStatus = "前回接続デバイスに再接続中..."
        }
        
        // 保存されたUUIDでデバイスを取得
        let knownPeripherals = centralManager.retrievePeripherals(withIdentifiers: [savedUUID])
        
        if let peripheral = knownPeripherals.first {
            print("🔄 保存されたデバイスに自動再接続試行: \(peripheral.name ?? "Unknown")")
            connect(to: peripheral)
        } else {
            print("🔄 保存されたデバイスが見つからない、スキャンを開始")
            isAutoReconnecting = false
            startScanning()
        }
    }
    
    // MARK: - Public Methods
    
    /// デバイスのスキャンを開始
    func startScanning() {
        startScanning(withServiceFilter: true)
    }
    
    /// デバイスのスキャンを開始（フィルター指定可能）
    func startScanning(withServiceFilter useFilter: Bool = true) {
        guard centralManager.state == .poweredOn else {
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetoothが無効です"
            }
            print("❌ Bluetooth無効でスキャン開始できません")
            return
        }
        
        print("🔍 スキャン開始 (フィルター: \(useFilter))")
        
        DispatchQueue.main.async {
            self.isScanning = true
            self.discoveredDevices.removeAll()
            self.connectionStatus = useFilter ? "デバイス検索中..." : "全デバイス検索中..."
        }
        
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
        DispatchQueue.main.async {
            self.isScanning = false
            if self.discoveredDevices.isEmpty && !self.isConnected {
                self.connectionStatus = "デバイスが見つかりません"
            }
        }
        print("🔍 スキャン停止")
    }
    
    /// 指定したデバイスに接続
    func connect(to peripheral: CBPeripheral) {
        stopScanning()
        DispatchQueue.main.async {
            self.connectionStatus = "接続中..."
            self.connectedPeripheral = peripheral
        }
        centralManager.connect(peripheral, options: nil)
        print("🔌 接続試行: \(peripheral.name ?? "Unknown")")
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

        // Debug: log the exact JSON payload being sent so we can verify on Xcode console
        print("🔤 Sending JSON payload: \(jsonString)")
        print("🔤 JSON length: \(jsonString.count) bytes")
        
        let data = jsonString.data(using: .utf8)!
        print("🔤 Data length: \(data.count) bytes")
        
        // Check MTU to ensure single packet transmission
        let mtu = peripheral.maximumWriteValueLength(for: .withResponse)
        print("🔤 MTU for withResponse: \(mtu) bytes")
        
        if data.count > mtu {
            print("⚠️ Payload (\(data.count) bytes) exceeds MTU (\(mtu) bytes) - may fragment")
            // Try with withoutResponse for larger MTU
            let mtuWithoutResponse = peripheral.maximumWriteValueLength(for: .withoutResponse)
            print("🔤 MTU for withoutResponse: \(mtuWithoutResponse) bytes")
            if data.count <= mtuWithoutResponse {
                print("✅ Using withoutResponse to avoid fragmentation")
                peripheral.writeValue(data, for: characteristic, type: .withoutResponse)
            } else {
                print("❌ Payload too large even for withoutResponse")
                return
            }
        } else {
            // Debug: print data as hex for verification
            let hexString = data.map { String(format: "%02x", $0) }.joined()
            print("📤 Sending data (\(data.count) bytes):")
            print("   JSON: \(jsonString)")
            print("   HEX: \(hexString)")
            peripheral.writeValue(data, for: characteristic, type: .withResponse)
        }
        
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
        print("🔄 Bluetooth状態変更: \(central.state.rawValue)")
        switch central.state {
        case .poweredOn:
            DispatchQueue.main.async {
                self.connectionStatus = "準備完了"
            }
            print("✅ Bluetooth準備完了")
            // 自動再接続を試行
            DispatchQueue.main.asyncAfter(deadline: .now() + 1) {
                self.attemptAutoReconnection()
            }
        case .poweredOff:
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetoothがオフです"
                self.isConnected = false
                self.isScanning = false
            }
            print("❌ Bluetoothがオフ")
        case .unauthorized:
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetooth使用が許可されていません"
                self.isConnected = false
                self.isScanning = false
            }
            print("❌ Bluetooth使用許可なし")
        case .unsupported:
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetoothがサポートされていません"
            }
            print("❌ Bluetoothサポートなし")
        case .resetting:
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetoothリセット中..."
                self.isConnected = false
                self.isScanning = false
            }
            print("⚠️ Bluetoothリセット中")
        default:
            DispatchQueue.main.async {
                self.connectionStatus = "Bluetooth状態不明"
            }
            print("⚠️ Bluetooth状態不明: \(central.state)")
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral, advertisementData: [String : Any], rssi RSSI: NSNumber) {
        // すでに発見済みかチェック
        guard !discoveredDevices.contains(where: { $0.identifier == peripheral.identifier }) else {
            return
        }
        
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
        
        // メインスレッドでUI更新
        DispatchQueue.main.async {
            // 全デバイス検索モードかどうかをチェック
            let isFullScanMode = self.connectionStatus.contains("全デバイス検索中")
            
            if isFullScanMode {
                // 全デバイススキャンモードの場合は全て追加
                self.discoveredDevices.append(peripheral)
                print("   📱 全デバイススキャンでデバイスを追加")
            } else if isKeyboardGW {
                // 通常スキャンでもKeyboardGWは追加（名前ベースフィルタリング）
                self.discoveredDevices.append(peripheral)
                print("   📱 通常スキャンでKeyboardGWデバイスを追加")
            }
        }
    }
    
    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        print("✅ 接続成功：\(peripheral.name ?? "Unknown")")
        
        // デバイス情報を保存（自動再接続用）
        savePeripheral(peripheral)
        isAutoReconnecting = false
        
        // メインスレッドでUI更新
        DispatchQueue.main.async {
            self.isConnected = true
            self.deviceName = peripheral.name ?? "KeyboardGW"
            self.connectionStatus = "接続済み"
        }
        
        peripheral.delegate = self
        peripheral.discoverServices([serviceUUID])
        
        // 3秒後にテスト接続確認を送信
        DispatchQueue.main.asyncAfter(deadline: .now() + 3) {
            self.sendConnectionTest()
        }
    }
    
    /// 接続テストメッセージを送信（デバイス側のLED更新を促すため）
    private func sendConnectionTest() {
        guard let characteristic = shortcutCharacteristic,
              let peripheral = connectedPeripheral,
              peripheral.state == .connected else {
            print("❌ 接続テスト失敗：デバイスが接続されていません")
            return
        }
        
        // Log MTU information after connection
        let mtuWithResponse = peripheral.maximumWriteValueLength(for: .withResponse)
        let mtuWithoutResponse = peripheral.maximumWriteValueLength(for: .withoutResponse)
        print("📊 MTU情報 - withResponse: \(mtuWithResponse), withoutResponse: \(mtuWithoutResponse)")
        
        let testCommand = [
            "keys": [""],  // 空のキー（実際には何も送信しない）
            "keyCount": 0,
            "test": true
        ] as [String : Any]
        
        guard let jsonData = try? JSONSerialization.data(withJSONObject: testCommand),
              let jsonString = String(data: jsonData, encoding: .utf8) else {
            print("❌ 接続テストJSONデータの作成に失敗")
            return
        }
        
        print("🔤 Test JSON: \(jsonString)")
        let data = jsonString.data(using: .utf8)!
        peripheral.writeValue(data, for: characteristic, type: .withoutResponse)
        print("📤 接続テストメッセージを送信")
    }
    
    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        print("❌ 接続失敗：\(error?.localizedDescription ?? "Unknown error")")
        
        isAutoReconnecting = false
        
        // メインスレッドでUI更新
        DispatchQueue.main.async {
            self.connectionStatus = "接続失敗"
            self.connectedPeripheral = nil
        }
    }
    
    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        print("🔌 デバイス切断：\(peripheral.name ?? "Unknown")")
        
        // メインスレッドでUI更新
        DispatchQueue.main.async {
            self.isConnected = false
            self.connectionStatus = "未接続"
            self.deviceName = ""
            self.connectedPeripheral = nil
            self.shortcutCharacteristic = nil
            self.statusCharacteristic = nil
            self.pairingCharacteristic = nil
        }
        
        if let error = error {
            print("❌ 切断エラー：\(error.localizedDescription)")
            // 予期しない切断の場合は自動再接続を試行
            DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
                self.attemptAutoReconnection()
            }
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
