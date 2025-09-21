import SwiftUI

struct SettingsView: View {
    @ObservedObject var store: ShortcutStore
    @Environment(\.presentationMode) private var presentationMode
    @State private var showExportAlert: Bool = false
    @State private var exportMessage: String = ""

    var body: some View {
        Form {
            // Export at top
            Section {
                HStack {
                    Button("再スキャン") {
                        store.refreshAvailableJsons()
                    }
                    Spacer()
                }

                Button("エクスポート (JSON)") {
                    doExport()
                }
                .foregroundColor(.blue)
                .alert(isPresented: $showExportAlert) {
                    Alert(title: Text("エクスポート"), message: Text(exportMessage), dismissButton: .default(Text("OK")))
                }
            }

            // JSON file selection list
            Section(header: Text("ショートカットJSON")) {
                // debug: show count and filenames
                if store.availableJsons.count > 0 {
                    Text("検出数: \(store.availableJsons.count)")
                        .font(.caption)
                        .foregroundColor(.secondary)
                }

                if store.availableJsons.isEmpty {
                    Text("利用可能なJSONが見つかりません。")
                        .foregroundColor(.secondary)
                } else {
                    ForEach(store.availableJsons) { entry in
                        HStack {
                            Text(entry.displayName)
                            Spacer()
                            Toggle(isOn: Binding<Bool>(
                                get: { store.selectedJsonFiles.contains(entry.fileName) },
                                set: { newValue in
                                    store.toggleJsonSelection(entry.fileName)
                                }
                            )) {
                                EmptyView()
                            }
                            .labelsHidden()
                        }
                    }
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
        let modifiedApps: [ShortcutApp] = store.apps.map { app in
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

            return ShortcutApp(id: app.id, appName: app.appName, disEnable: app.disEnable, order: app.order, icon: app.icon, version: app.version, groups: modifiedGroups)
        }

        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]
        do {
            let data = try encoder.encode(modifiedApps)
            if let jsonString = String(data: data, encoding: .utf8) {
                UIPasteboard.general.string = jsonString
                exportMessage = "JSONをクリップボードにコピーしました。"
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
