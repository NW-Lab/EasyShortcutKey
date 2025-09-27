import SwiftUI
import CoreBluetooth

struct KeyboardGWPairingView: View {
    @ObservedObject private var keyboardGWManager = KeyboardGWManager.shared
    @Environment(\.presentationMode) private var presentationMode
    
    var body: some View {
        NavigationView {
            VStack(spacing: 20) {
                // ヘッダー部分
                VStack(spacing: 12) {
                    Image(systemName: "keyboard")
                        .font(.system(size: 60))
                        .foregroundColor(.blue)
                    
                    Text("KeyboardGW")
                        .font(.title2)
                        .fontWeight(.bold)
                    
                    Text("外部キーボードデバイスと接続してショートカットキーを送信できます")
                        .font(.caption)
                        .multilineTextAlignment(.center)
                        .foregroundColor(.secondary)
                }
                .padding()
                .background(Color(.systemGray6))
                .cornerRadius(12)
                
                // 接続状況表示
                VStack(spacing: 8) {
                    HStack {
                        Circle()
                            .fill(keyboardGWManager.isConnected ? Color.green : Color.gray)
                            .frame(width: 12, height: 12)
                        
                        Text(keyboardGWManager.connectionStatus)
                            .font(.subheadline)
                            .fontWeight(.medium)
                        
                        Spacer()
                        
                        if keyboardGWManager.isConnected && keyboardGWManager.batteryLevel >= 0 {
                            HStack(spacing: 4) {
                                Image(systemName: batteryIcon)
                                    .foregroundColor(batteryColor)
                                Text("\(keyboardGWManager.batteryLevel)%")
                                    .font(.caption)
                                    .foregroundColor(.secondary)
                            }
                        }
                    }
                    
                    if keyboardGWManager.isConnected {
                        Text("デバイス名: \(keyboardGWManager.deviceName)")
                            .font(.caption)
                            .foregroundColor(.secondary)
                    }
                }
                .padding()
                .background(Color(.systemBackground))
                .cornerRadius(8)
                .overlay(
                    RoundedRectangle(cornerRadius: 8)
                        .stroke(Color(.separator), lineWidth: 1)
                )
                
                // スキャン・接続ボタン
                if !keyboardGWManager.isConnected {
                    VStack(spacing: 16) {
                        Button(action: {
                            keyboardGWManager.startScanning()
                        }) {
                            HStack {
                                if keyboardGWManager.isScanning {
                                    ProgressView()
                                        .progressViewStyle(CircularProgressViewStyle(tint: .white))
                                        .scaleEffect(0.8)
                                } else {
                                    Image(systemName: "magnifyingglass")
                                }
                                Text(keyboardGWManager.isScanning ? "検索中..." : "デバイスを検索")
                                    .fontWeight(.semibold)
                            }
                            .foregroundColor(.white)
                            .frame(maxWidth: .infinity)
                            .padding()
                            .background(keyboardGWManager.isScanning ? Color.orange : Color.blue)
                            .cornerRadius(12)
                        }
                        .disabled(keyboardGWManager.isScanning)
                        
                        // 発見されたデバイス一覧
                        if !keyboardGWManager.discoveredDevices.isEmpty {
                            VStack(alignment: .leading, spacing: 8) {
                                Text("発見されたデバイス")
                                    .font(.headline)
                                    .padding(.leading, 4)
                                
                                ForEach(keyboardGWManager.discoveredDevices, id: \.identifier) { device in
                                    Button(action: {
                                        keyboardGWManager.connect(to: device)
                                    }) {
                                        HStack {
                                            Image(systemName: "keyboard")
                                                .foregroundColor(.blue)
                                            
                                            VStack(alignment: .leading, spacing: 2) {
                                                Text(device.name ?? "Unknown Device")
                                                    .fontWeight(.medium)
                                                    .foregroundColor(.primary)
                                                Text(device.identifier.uuidString.prefix(8))
                                                    .font(.caption)
                                                    .foregroundColor(.secondary)
                                            }
                                            
                                            Spacer()
                                            
                                            Image(systemName: "chevron.right")
                                                .foregroundColor(.secondary)
                                                .font(.caption)
                                        }
                                        .padding()
                                        .background(Color(.systemGray6))
                                        .cornerRadius(8)
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // 接続済みの場合の操作ボタン
                    VStack(spacing: 12) {
                        Button("テストキー送信") {
                            keyboardGWManager.sendShortcut(keys: ["cmd", "c"])
                        }
                        .foregroundColor(.white)
                        .frame(maxWidth: .infinity)
                        .padding()
                        .background(Color.green)
                        .cornerRadius(12)
                        
                        Button("切断") {
                            keyboardGWManager.disconnect()
                        }
                        .foregroundColor(.white)
                        .frame(maxWidth: .infinity)
                        .padding()
                        .background(Color.red)
                        .cornerRadius(12)
                    }
                }
                
                Spacer()
                
                // 説明文
                Text("KeyboardGWデバイスをペアリングすると、ショートカット表示時に実際のキー入力が送信されます。")
                    .font(.caption)
                    .multilineTextAlignment(.center)
                    .foregroundColor(.secondary)
                    .padding()
            }
            .padding()
            .navigationTitle("KeyboardGW設定")
            .navigationBarTitleDisplayMode(.inline)
            .navigationBarBackButtonHidden(true)
            .toolbar {
                ToolbarItem(placement: .navigationBarTrailing) {
                    Button("完了") {
                        presentationMode.wrappedValue.dismiss()
                    }
                }
            }
        }
    }
    
    // バッテリーアイコンの計算
    private var batteryIcon: String {
        let level = keyboardGWManager.batteryLevel
        if level >= 75 {
            return "battery.100"
        } else if level >= 50 {
            return "battery.75"
        } else if level >= 25 {
            return "battery.25"
        } else {
            return "battery.0"
        }
    }
    
    // バッテリーレベルによる色の計算
    private var batteryColor: Color {
        let level = keyboardGWManager.batteryLevel
        if level >= 50 {
            return .green
        } else if level >= 25 {
            return .orange
        } else {
            return .red
        }
    }
}

#Preview {
    KeyboardGWPairingView()
}
