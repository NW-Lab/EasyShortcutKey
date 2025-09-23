import SwiftUI

struct SettingsView: View {
    @ObservedObject var store: ShortcutStore
    @Environment(\.presentationMode) private var presentationMode
    @State private var showExportAlert: Bool = false
    @State private var exportMessage: String = ""

    var body: some View {
        Form {
            // KeyboardGW設定セクション
            Section(header: Text("外部デバイス")) {
                NavigationLink(destination: KeyboardGWPairingView()) {
                    HStack {
                        Image(systemName: "keyboard")
                            .foregroundColor(.blue)
                            .frame(width: 24)
                        VStack(alignment: .leading, spacing: 2) {
                            Text("KeyboardGW")
                                .font(.body)
                            Text("PCのUSBキーボードとして認識されます。")
                                .font(.caption)
                                .foregroundColor(.secondary)
                        }
                        Spacer()
                    }
                }
            }
            
            // Export at top
            Section {
                Button("エクスポート (JSON)") {
                    doExport()
                }
                .foregroundColor(.blue)
                .alert(isPresented: $showExportAlert) {
                    Alert(title: Text("エクスポート"), message: Text(exportMessage), dismissButton: .default(Text("OK")))
                }
            }
            
            // Integrated app management section
            Section(header: Text("アプリ管理（表示・順序）")) {
                let allApps = store.getAllAvailableApps()
                
                if allApps.isEmpty {
                    Text("アプリが見つかりません。")
                        .foregroundColor(.secondary)
                } else {
                    ForEach(allApps, id: \.id) { app in
                        HStack {
                            Image(systemName: "line.horizontal.3")
                                .foregroundColor(.secondary)
                            Text(app.appName)
                            Spacer()
                            Toggle(isOn: Binding<Bool>(
                                get: { !store.hiddenApps.contains(app.appName) },
                                set: { newValue in
                                    store.toggleAppVisibility(app.appName)
                                }
                            )) {
                                EmptyView()
                            }
                            .labelsHidden()
                        }
                        .padding(.vertical, 2)
                        .opacity(store.hiddenApps.contains(app.appName) ? 0.5 : 1.0)
                    }
                    .onMove(perform: { source, destination in
                        let allAppNames = allApps.map { $0.appName }
                        var newOrder = store.customAppOrder.isEmpty ? allAppNames : store.customAppOrder
                        
                        // Ensure all current apps are in the order array
                        for appName in allAppNames {
                            if !newOrder.contains(appName) {
                                newOrder.append(appName)
                            }
                        }
                        
                        newOrder.move(fromOffsets: source, toOffset: destination)
                        store.customAppOrder = newOrder
                    })
                    
                    Text("ドラッグして並び替え、トグルで表示/非表示を設定できます。")
                        .font(.caption)
                        .foregroundColor(.secondary)
                }
            }
        }
        .navigationTitle("設定")
        .onAppear {
            // refresh when view appears so newly added bundle resources are detected during testing
            store.refreshAvailableJsons()
        }
        .navigationBarBackButtonHidden(true)
        .toolbar {
            ToolbarItem(placement: .navigationBarTrailing) {
                Button(action: { presentationMode.wrappedValue.dismiss() }) {
                    Text("閉じる")
                }
            }
        }
    }
}

extension SettingsView {
    private func doExport() {
        // Build modified apps where shortcuts in store.hiddenIDs get disEnable = true
        // and update order based on custom app ordering
        let modifiedApps: [ShortcutApp] = store.filteredApps.enumerated().map { (index, app) in
            let modifiedGroups = app.groups?.map { group in
                let modifiedShortcuts = group.shortcuts?.map { shortcut -> ShortcutItem in
                    if store.hiddenIDs.contains(shortcut.id) {
                        return ShortcutItem(id: shortcut.id, action: shortcut.action, disEnable: true, keys: shortcut.keys, steps: shortcut.steps, description: shortcut.description, context: shortcut.context, order: shortcut.order)
                    } else {
                        return shortcut
                    }
                }
                return ShortcutGroup(id: group.id, groupName: group.groupName, disEnable: group.disEnable, order: group.order, description: group.description, shortcuts: modifiedShortcuts)
            }

            // Update order based on current position in filteredApps (which reflects custom ordering)
            return ShortcutApp(id: app.id, appName: app.appName, disEnable: app.disEnable, order: index, icon: app.icon, version: app.version, groups: modifiedGroups)
        }

        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        do {
            let data = try encoder.encode(modifiedApps)
            if let jsonString = String(data: data, encoding: .utf8) {
                UIPasteboard.general.string = jsonString
                exportMessage = "アプリデータ(\(modifiedApps.count)個)をクリップボードにコピーしました。"
                showExportAlert = true
            }
        } catch {
            exportMessage = "エクスポートに失敗しました: \(error)"
            showExportAlert = true
        }
    }
}

struct SettingsView_Previews: PreviewProvider {
    static var previews: some View {
        SettingsView(store: ShortcutStore())
    }
}
