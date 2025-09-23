import SwiftUI
import SwiftData
import UIKit

struct ContentView: View {
    @StateObject private var store = ShortcutStore()
    @State private var copiedText: String = ""
    @State private var showCopyFeedback: Bool = false

    var body: some View {
        NavigationView {
            VStack(spacing: 0) {
                // Header with title on first line, toggle + settings button on second line
                VStack(alignment: .leading, spacing: 4) {
                    // 1行目: タイトル
                    //Text("Easy Shortcut Key")
                    //    .font(.largeTitle)
                    //    .bold()
                    
                    // 2行目: トグルと設定ボタン
                    HStack {
                        Toggle("非表示を表示", isOn: $store.showHidden)
                            .toggleStyle(SwitchToggleStyle())
                            .font(.caption)
                            .frame(width: 150)
                        Spacer()
                        
                        // NavigationLink で設定画面へ遷移
                        NavigationLink(destination: SettingsView(store: store)) {
                            HStack(spacing: 6) {
                                Image(systemName: "gearshape")
                                Text("設定")
                                    .font(.caption)
                            }
                            .padding(6)
                        }
                        .buttonStyle(BorderlessButtonStyle())
                    }
                }
                // 横パディングは確保しつつ下の余白を小さくする
                .padding(.top, 16)
                .padding(.horizontal)
                .padding(.bottom, 2)
                .background(Color(UIColor.systemGray6))
                
                // Main content
                // App タブ（複数アプリを選択）
                if store.filteredApps.count > 0 {
                    let safeAppIndex = min(max(0, store.selectedAppIndex), store.filteredApps.count - 1)

                    // アプリ名をタブで表示
                    // App tabs: horizontally scrollable buttons to avoid truncation
                    ScrollView(.horizontal, showsIndicators: false) {
                        HStack(spacing: 8) {
                            ForEach(Array(store.filteredApps.enumerated()), id: \.offset) { idx, app in
                                Button(action: { store.selectedAppIndex = idx }) {
                                    Text(app.appName)
                                        .font(.subheadline)
                                        .lineLimit(1)
                                        .padding(.vertical, 6)
                                        .padding(.horizontal, 12)
                                        .background(store.selectedAppIndex == idx ? Color.accentColor.opacity(0.18) : Color(UIColor.systemGray6))
                                        .foregroundColor(store.selectedAppIndex == idx ? Color.primary : Color.primary)
                                        .cornerRadius(8)
                                }
                                .buttonStyle(BorderlessButtonStyle())
                            }
                        }
                        .padding(.horizontal)
                        .padding(.vertical, 2)
                    }

                    // 選択中アプリのみ表示
                    List {
                        let app = store.filteredApps[safeAppIndex]
                        Section(header: appSectionHeader(app)) {
                            if let groups = app.groups, groups.count > 0 {
                                // 初期選択をセット（safeIndex は折りたたみ時のみ使用するので _ に置換）
                                //let currentIndexOptional = store.selectedGroupIndex[app.id] ?? nil
                                //let currentIndex = currentIndexOptional ?? 0

                                // グループタブと展開ボタン
                                HStack(alignment: .center, spacing: 6) {
                                    Button(action: {
                                        // toggle expanded state in set
                                        if store.expandedSet.contains(app.id) {
                                            store.expandedSet.remove(app.id)
                                            // when collapsing, restore selection to 0 if nil
                                            if store.selectedGroupIndex[app.id] == nil {
                                                store.selectedGroupIndex[app.id] = 0
                                            }
                                        } else {
                                            store.expandedSet.insert(app.id)
                                            // when expanding, clear selection
                                            store.selectedGroupIndex[app.id] = nil
                                        }
                                    }) {
                                        Image(systemName: (store.expandedSet.contains(app.id)) ? "chevron.down" : "chevron.right")
                                            .font(.system(size: 16, weight: .semibold))
                                            .frame(width: 24, height: 24)
                                    }
                                    .padding(.horizontal, 2)
                                    .frame(minWidth: 40, minHeight: 24, maxHeight: 24)
                                    .background(Color(UIColor.systemGray6))
                                    .overlay(
                                        RoundedRectangle(cornerRadius: 16)
                                            .stroke(Color(UIColor.separator).opacity(0.7), lineWidth: 0.8)
                                    )
                                    .cornerRadius(16)
                                    .buttonStyle(BorderlessButtonStyle())

                                    // グループタブ（optional selection allowed）
                                    // Group tabs: horizontally scrollable buttons (collapsed mode)
                                    ScrollView(.horizontal, showsIndicators: false) {
                                        HStack(spacing: 8) {
                                            ForEach(Array(groups.enumerated()), id: \.offset) { idx, group in
                                                let selectedIndex = store.selectedGroupIndex[app.id] ?? 0
                                                let isSelected = selectedIndex == idx
                                                Button(action: {
                                                    store.selectedGroupIndex[app.id] = idx
                                                    store.expandedSet.remove(app.id)
                                                }) {
                                                    Text(group.groupName)
                                                        .font(.caption)
                                                        .lineLimit(1)
                                                        .padding(.vertical, 6)
                                                        .padding(.horizontal, 10)
                                                        .background(isSelected ? Color.accentColor.opacity(0.18) : Color(UIColor.systemGray6))
                                                        .cornerRadius(8)
                                                }
                                                .buttonStyle(BorderlessButtonStyle())
                                                .disabled(store.expandedSet.contains(app.id))
                                            }
                                        }
                                        .frame(height: 32)
                                        .layoutPriority(1)
                                    }
                                 }
                                 
                                // 展開時は全グループを表示、グループ名を出力
                                if store.expandedSet.contains(app.id) {
                                     ForEach(groups) { group in
                                         // グループ名（目立つ装飾）
                                         HStack(spacing: 8) {
                                             Image(systemName: "folder.fill")
                                                 .foregroundColor(.accentColor)
                                                 .font(.caption)
                                              
                                             Text(group.groupName)
                                                 .font(.headline)
                                                 .fontWeight(.semibold)
                                         }
                                         .padding(.horizontal, 12)
                                         .padding(.vertical, 6)
                                         .background(Color.accentColor.opacity(0.1))
                                         .overlay(
                                             RoundedRectangle(cornerRadius: 8)
                                                 .stroke(Color.accentColor.opacity(0.3), lineWidth: 1)
                                         )
                                         .cornerRadius(8)
                                         .padding(.top, 2)

                                        if let shortcuts = group.shortcuts {
                                            ForEach(shortcuts) { item in
                                                shortcutRow(item)
                                                    .opacity(item.disEnable == true ? 0.5 : 1.0)
                                            }
                                        }
                                     }
                                } else {
                                    // 折りたたみ時は選択中のグループのみ表示
                                    let selIdxOptional = store.selectedGroupIndex[app.id] ?? nil
                                    let selIdx = selIdxOptional ?? 0
                                    let safeIdx = min(max(0, selIdx), groups.count - 1)
                                     if let shortcuts = groups[safeIdx].shortcuts {
                                         ForEach(shortcuts) { item in
                                             shortcutRow(item)
                                                 .opacity(item.disEnable == true ? 0.5 : 1.0)
                                                 .swipeActions(edge: .trailing) {
                                                     Button {
                                                         toggleHidden(item.id)
                                                     } label: {
                                                         if store.hiddenIDs.contains(item.id) {
                                                             Label("表示", systemImage: "eye")
                                                         } else {
                                                             Label("非表示", systemImage: "eye.slash")
                                                         }
                                                     }
                                                     .tint(store.hiddenIDs.contains(item.id) ? .green : .gray)
                                                 }
                                         }
                                     }
                                }
                              }
                          }
                      }
                    .listStyle(PlainListStyle())
                 } else {
                    // apps が空のときは空表示
                    List { Text("No apps") }
                 }
                
                // Copy feedback overlay
                if showCopyFeedback {
                    VStack {
                        Spacer()
                        HStack {
                            Image(systemName: "doc.on.clipboard.fill")
                            Text("コピーしました: \(copiedText)")
                        }
                        .padding()
                        .background(Color.black.opacity(0.8))
                        .foregroundColor(.white)
                        .cornerRadius(10)
                        .transition(.opacity)
                    }
                    .padding()
                }
            }
        }
        .onAppear(perform: onAppearLoad)
        .navigationViewStyle(StackNavigationViewStyle())
    }
    
    private func appSectionHeader(_ app: ShortcutApp) -> some View {
        // App名は上部のタブで表示しているため、ここではヘッダを表示しない
        EmptyView()
    }
    
    private func shortcutRow(_ item: ShortcutItem) -> some View {
         HStack(alignment: .top, spacing: 8) {
            VStack(alignment: .leading, spacing: 2) {
                // Action と description を1行に収める（長い場合は縮小してフィット）
                HStack(spacing: 6) {
                    Text(item.action)
                        .font(.headline)
                        .lineLimit(1)
                        .minimumScaleFactor(0.75)

                    Text("—")
                        .font(.subheadline)

                    Text(item.description)
                        .font(.subheadline)
                        .foregroundColor(.secondary)
                        .lineLimit(1)
                        .minimumScaleFactor(0.75)
                }

                if let context = item.context {
                    Text("Context: \(context)")
                        .font(.caption2)
                        .foregroundColor(.orange)
                }
            }
            
            Spacer()
            
            // Display keys or steps
            VStack(alignment: .trailing, spacing: 4) {
                if let keys = item.keys {
                    // Simple key combination
                    keysButton(keys: keys)
                } else if let steps = item.steps {
                    // Multi-step shortcut
                    VStack(alignment: .trailing, spacing: 2) {
                        ForEach(Array(steps.enumerated()), id: \.offset) { index, step in
                            stepView(step: step, index: index + 1)
                        }
                    }
                }
            }
        }
        .padding(.vertical, 2)
    }
    
    private func keysButton(keys: [String]) -> some View {
        Button(action: {
            //copyToClipboard(keys.joined(separator: " + "))
        }) {
            Text(keys.joined(separator: " + "))
                .font(.caption)
                .padding(.horizontal, 8)
                .padding(.vertical, 2)
                .background(Color.blue.opacity(0.15))
                .foregroundColor(.blue)
                .cornerRadius(6)
        }
    }
    
    private func stepView(step: ShortcutStep, index: Int) -> some View {
        HStack(spacing: 4) {
            Text("\(index).")
                .font(.caption2)
                .foregroundColor(.secondary)
            
            if let keys = step.keys {
                Button(action: {
                    //copyToClipboard(keys.joined(separator: " + "))
                }) {
                    Text(keys.joined(separator: " + "))
                        .font(.caption2)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 1)
                        .background(Color.green.opacity(0.15))
                        .foregroundColor(.green)
                        .cornerRadius(4)
                }
            } else if let action = step.action {
                Text(action)
                    .font(.caption2)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 1)
                    .background(Color.orange.opacity(0.15))
                    .foregroundColor(.orange)
                    .cornerRadius(4)
            } else {
                Text(step.description ?? "Unknown step")
                    .font(.caption2)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 1)
                    .background(Color.gray.opacity(0.15))
                    .foregroundColor(.gray)
                    .cornerRadius(4)
            }
        }
    }
    
    private func copyToClipboard(_ text: String) {
        UIPasteboard.general.string = text
        copiedText = text
        
        withAnimation(.easeInOut(duration: 0.3)) {
            showCopyFeedback = true
        }
        
        DispatchQueue.main.asyncAfter(deadline: .now() + 2) {
            withAnimation(.easeInOut(duration: 0.3)) {
                showCopyFeedback = false
            }
        }
    }
    
    @Environment(\.modelContext) private var modelContext

    private func loadHiddenIDs() {
        // Fetch HiddenShortcut entities and populate store.hiddenIDs
        let fetch = FetchDescriptor<HiddenShortcut>()
        if let saved: [HiddenShortcut] = try? modelContext.fetch(fetch) {
            let ids = Set(saved.map { $0.id })
            store.hiddenIDs = ids
        }
    }

    private func toggleHidden(_ id: UUID) {
        if store.hiddenIDs.contains(id) {
            // unhide: delete entity
            let fetch = FetchDescriptor<HiddenShortcut>(predicate: #Predicate { $0.id == id })
            if let objs: [HiddenShortcut] = try? modelContext.fetch(fetch) {
                for o in objs { modelContext.delete(o) }
            }
            store.hiddenIDs.remove(id)
        } else {
            // hide: create entity
            let h = HiddenShortcut(id: id)
            modelContext.insert(h)
            store.hiddenIDs.insert(id)
        }
    }

    // Load hidden IDs when view appears
    private func onAppearLoad() {
        loadHiddenIDs()
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
