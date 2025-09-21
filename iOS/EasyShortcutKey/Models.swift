import Foundation
import Combine

// MARK: - Step Models for multi-step shortcuts
struct ShortcutStep: Codable, Identifiable {
    let id = UUID()
    let type: String  // "keys", "input", "ui", "wait", "macroRef"
    let keys: [String]?
    let action: String?
    let description: String?
    let duration: Int?
    
    private enum CodingKeys: String, CodingKey {
        case type, keys, action, description, duration
    }
}

// MARK: - Main Models
struct ShortcutApp: Codable, Identifiable {
    let id: UUID
    let appName: String
    let disEnable: Bool?
    let order: Int?
    let icon: String?
    let version: String?
    let groups: [ShortcutGroup]?
    
    private enum CodingKeys: String, CodingKey {
        case id, appName, disEnable, order, icon, version, groups
    }
    
    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        
        // Handle id field - use existing or generate new UUID
        if let idString = try? container.decode(String.self, forKey: .id),
           let uuid = UUID(uuidString: idString) {
            self.id = uuid
        } else {
            self.id = UUID()
        }
        
        self.appName = try container.decode(String.self, forKey: .appName)
        self.disEnable = try? container.decode(Bool.self, forKey: .disEnable)
        self.order = try? container.decode(Int.self, forKey: .order)
        self.icon = try? container.decode(String.self, forKey: .icon)
        self.version = try? container.decode(String.self, forKey: .version)
        self.groups = try? container.decode([ShortcutGroup].self, forKey: .groups)
    }
}

struct ShortcutGroup: Codable, Identifiable {
    let id: UUID
    let groupName: String
    let disEnable: Bool?
    let order: Int?
    let description: String?
    let shortcuts: [ShortcutItem]?
    
    private enum CodingKeys: String, CodingKey {
        case id, groupName, disEnable, order, description, shortcuts
    }
    
    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        
        // Handle id field - use existing or generate new UUID
        if let idString = try? container.decode(String.self, forKey: .id),
           let uuid = UUID(uuidString: idString) {
            self.id = uuid
        } else {
            self.id = UUID()
        }
        
        self.groupName = try container.decode(String.self, forKey: .groupName)
        self.disEnable = try? container.decode(Bool.self, forKey: .disEnable)
        self.order = try? container.decode(Int.self, forKey: .order)
        self.description = try? container.decode(String.self, forKey: .description)
        self.shortcuts = try? container.decode([ShortcutItem].self, forKey: .shortcuts)
    }
}

struct ShortcutItem: Codable, Identifiable {
    let id: UUID
    let action: String
    let disEnable: Bool?
    let keys: [String]?
    let steps: [ShortcutStep]?
    let description: String
    let context: String?
    let order: Int?
    
    private enum CodingKeys: String, CodingKey {
        case id, action, disEnable, keys, steps, description, context, order
    }
    
    init(from decoder: Decoder) throws {
        let container = try decoder.container(keyedBy: CodingKeys.self)
        
        // Handle id field - use existing or generate new UUID
        if let idString = try? container.decode(String.self, forKey: .id),
           let uuid = UUID(uuidString: idString) {
            self.id = uuid
        } else {
            self.id = UUID()
        }
        
        self.action = try container.decode(String.self, forKey: .action)
        self.disEnable = try? container.decode(Bool.self, forKey: .disEnable)
        self.keys = try? container.decode([String].self, forKey: .keys)
        self.steps = try? container.decode([ShortcutStep].self, forKey: .steps)
        self.description = try container.decode(String.self, forKey: .description)
        self.context = try? container.decode(String.self, forKey: .context)
        self.order = try? container.decode(Int.self, forKey: .order)
    }
}

class ShortcutStore: ObservableObject {
    @Published var apps: [ShortcutApp] = []
    @Published var filteredApps: [ShortcutApp] = []
    @Published var showHidden: Bool = false {
        didSet {
            updateFilteredApps()
        }
    }
    // Hidden shortcut UUIDs (persisted via SwiftData elsewhere). Stored here to apply filtering.
    @Published var hiddenIDs: Set<UUID> = [] {
        didSet { updateFilteredApps() }
    }

    // MARK: - JSON file discovery & selection
    struct JsonFileEntry: Identifiable {
        let id = UUID()
        let fileName: String // filename with extension
        let displayName: String
        let url: URL
    }
    @Published var availableJsons: [JsonFileEntry] = []

    // Selected json filenames (persisted to UserDefaults)
    @Published var selectedJsonFiles: Set<String> = [] {
        didSet {
            UserDefaults.standard.set(Array(selectedJsonFiles), forKey: "SelectedShortcutJsonFiles")
            loadSelectedFiles()
        }
    }

    init(filename: String = "Shortcuts.json") {
        // discover bundled shortcutJsons
        scanShortcutJsons()

        // load persisted selection if any
        if let saved = UserDefaults.standard.stringArray(forKey: "SelectedShortcutJsonFiles"), saved.count > 0 {
            self.selectedJsonFiles = Set(saved)
            loadSelectedFiles()
        } else {
            // fallback to default single file
            load(from: filename)
        }
    }

    func load(from filename: String) {
        guard let url = Bundle.main.url(forResource: filename.replacingOccurrences(of: ".json", with: ""), withExtension: "json") else {
            print("Shortcut JSON not found in bundle: \(filename)")
            return
        }

        do {
            let data = try Data(contentsOf: url)
            let decoder = JSONDecoder()
            let decoded = try decoder.decode([ShortcutApp].self, from: data)
            DispatchQueue.main.async {
                self.apps = decoded
                self.updateFilteredApps()
            }
        } catch {
            print("Failed to load/parse shortcuts: \(error)")
        }
    }
    
    // Discover JSON files bundled in shortcutJsons directory and decode displayName
    private func scanShortcutJsons() {
        print("[ShortcutStore] scanShortcutJsons: start scanning bundle for JSON files (prefer shortcutJsons subdirectory)")

        var entries: [JsonFileEntry] = []
        let decoder = JSONDecoder()
        var seenNames: Set<String> = []

        // First, try to get files specifically in shortcutJsons subdirectory (preferred)
        if let subUrls = Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: "shortcutJsons"), subUrls.count > 0 {
            print("[ShortcutStore] found \(subUrls.count) files in shortcutJsons subdirectory")
            for url in subUrls {
                let fname = url.lastPathComponent
                guard !seenNames.contains(fname) else { continue }
                seenNames.insert(fname)

                print("[ShortcutStore] inspecting subdir file: \(url.path)")
                do {
                    let data = try Data(contentsOf: url)
                    do {
                        let apps = try decodeApps(from: data, using: decoder)
                        let display = apps.first?.appName ?? fname
                        entries.append(JsonFileEntry(fileName: fname, displayName: display, url: url))
                        print("[ShortcutStore] decoded JSON from subdir: \(fname) -> display=\(display)")
                    } catch {
                        print("[ShortcutStore] decode error for subdir file \(fname): \(error)")
                    }
                } catch {
                    print("[ShortcutStore] failed to read data at url: \(url.path) error: \(error)")
                }
            }

            // Deduplicate by normalized displayName to avoid multiple entries with same appName
            var uniqueByKey: [String: JsonFileEntry] = [:]
            for e in entries {
                let key = e.displayName.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
                if uniqueByKey[key] == nil {
                    uniqueByKey[key] = e
                } else {
                    print("[ShortcutStore] skipping duplicate displayName: \(e.displayName) at \(e.url.path)")
                }
            }
            availableJsons = uniqueByKey.values.sorted { $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending }
            print("[ShortcutStore] scan complete (subdir). found \(availableJsons.count) json entries (deduped)")
             return
        }

        // Fallback: recursive scan of bundle resources (used for testing), but exclude top-level Shortcuts.json
        guard let resourceURL = Bundle.main.resourceURL else {
            print("[ShortcutStore] Bundle.resourceURL is nil")
            availableJsons = []
            return
        }

        let fm = FileManager.default
        let enumerator = fm.enumerator(at: resourceURL, includingPropertiesForKeys: nil)

        while let element = enumerator?.nextObject() as? URL {
            if element.pathExtension.lowercased() != "json" { continue }
            let fname = element.lastPathComponent
            // skip the main Shortcuts.json at bundle root to avoid duplicate default
            if fname == "Shortcuts.json" { continue }
            if seenNames.contains(fname) { continue }
            seenNames.insert(fname)

            print("[ShortcutStore] inspecting file: \(element.path)")
            do {
                let data = try Data(contentsOf: element)
                do {
                    let apps = try decodeApps(from: data, using: decoder)
                    let display = apps.first?.appName ?? fname
                    entries.append(JsonFileEntry(fileName: fname, displayName: display, url: element))
                    print("[ShortcutStore] decoded JSON: \(fname) -> display=\(display) at \(element.path)")
                } catch {
                    print("[ShortcutStore] decode error for file \(fname): \(error)")
                }
            } catch {
                print("[ShortcutStore] failed to read data at url: \(element.path) error: \(error)")
            }
        }

        // Deduplicate by normalized displayName for fallback scan as well
        var uniqueByKeyFallback: [String: JsonFileEntry] = [:]
        for e in entries {
            let key = e.displayName.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
            if uniqueByKeyFallback[key] == nil {
                uniqueByKeyFallback[key] = e
            } else {
                print("[ShortcutStore] skipping duplicate displayName (fallback): \(e.displayName) at \(e.url.path)")
            }
        }
        availableJsons = uniqueByKeyFallback.values.sorted { $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending }
        print("[ShortcutStore] scan complete. found \(availableJsons.count) json entries (deduped)")
     }
     
     // Public refresh for UI to trigger re-scan
     func refreshAvailableJsons() {
         scanShortcutJsons()
         print("[ShortcutStore] refreshAvailableJsons -> \(availableJsons.map { $0.fileName })")
     }

     // Load apps from selected JSON files (merge results)
     func loadSelectedFiles() {
         var merged: [ShortcutApp] = []
         let decoder = JSONDecoder()
         for entry in availableJsons {
             if selectedJsonFiles.contains(entry.fileName) {
                 // use the discovered URL directly
                 do {
                     let data = try Data(contentsOf: entry.url)
                     do {
                         let decodedApps = try decodeApps(from: data, using: decoder)
                         merged.append(contentsOf: decodedApps)
                     } catch {
                         print("[ShortcutStore] decode error when loading selected file \(entry.fileName): \(error)")
                     }
                 } catch {
                     print("[ShortcutStore] failed to read selected file at url: \(entry.url.path) error: \(error)")
                 }
             }
         }

         DispatchQueue.main.async {
             self.apps = merged
             self.updateFilteredApps()
         }
     }

     // Toggle selection helper
     func toggleJsonSelection(_ fileName: String) {
         if selectedJsonFiles.contains(fileName) {
             selectedJsonFiles.remove(fileName)
         } else {
             selectedJsonFiles.insert(fileName)
         }
     }
    
    private func updateFilteredApps() {
        filteredApps = apps.compactMap { (app) -> ShortcutApp? in
            // Filter disabled apps unless showHidden is true
            if app.disEnable == true && !showHidden {
                return nil
            }

            // Filter groups within the app
            let filteredGroups = app.groups?.compactMap { (group) -> ShortcutGroup? in
                // Filter disabled groups unless showHidden is true
                if group.disEnable == true && !showHidden {
                    return nil
                }

                // Filter shortcuts within the group
                let filteredShortcuts = group.shortcuts?.compactMap { (shortcut) -> ShortcutItem? in
                    // Filter disabled shortcuts from JSON unless showHidden is true
                    if shortcut.disEnable == true && !showHidden {
                        return nil
                    }
                    // Filter user-hidden shortcuts (hiddenIDs) unless showHidden is true
                    if self.hiddenIDs.contains(shortcut.id) && !showHidden {
                        return nil
                    }
                    return shortcut
                }

                // Create new group with filtered shortcuts
                return ShortcutGroup(
                    id: group.id,
                    groupName: group.groupName,
                    disEnable: group.disEnable,
                    order: group.order,
                    description: group.description,
                    shortcuts: filteredShortcuts
                )
            }

            // Create new app with filtered groups
            return ShortcutApp(
                id: app.id,
                appName: app.appName,
                disEnable: app.disEnable,
                order: app.order,
                icon: app.icon,
                version: app.version,
                groups: filteredGroups
            )
        }

        // Sort apps by order
        filteredApps = sortByOrder(filteredApps) { $0.order }

        // Sort groups and shortcuts within each app
        for i in filteredApps.indices {
            if let groups = filteredApps[i].groups {
                let sortedGroups = sortByOrder(groups) { $0.order }
                for j in sortedGroups.indices {
                    if let shortcuts = sortedGroups[j].shortcuts {
                        _ = sortByOrder(shortcuts) { $0.order }
                        // This is a bit complex due to the struct immutability
                        // For now, we'll leave the sorting to the UI layer
                    }
                }
            }
        }
    }
    
    private func sortByOrder<T>(_ items: [T], orderSelector: (T) -> Int?) -> [T] {
        return items.sorted { lhs, rhs in
            let lhsOrder = orderSelector(lhs)
            let rhsOrder = orderSelector(rhs)
            
            switch (lhsOrder, rhsOrder) {
            case let (l?, r?):
                return l < r
            case (_?, nil):
                return true
            case (nil, _?):
                return false
            case (nil, nil):
                return false
            }
        }
    }
}

// MARK: - Helper Extensions
extension ShortcutGroup {
    init(id: UUID, groupName: String, disEnable: Bool?, order: Int?, description: String?, shortcuts: [ShortcutItem]?) {
        self.id = id
        self.groupName = groupName
        self.disEnable = disEnable
        self.order = order
        self.description = description
        self.shortcuts = shortcuts
    }
}

extension ShortcutApp {
    init(id: UUID, appName: String, disEnable: Bool?, order: Int?, icon: String?, version: String?, groups: [ShortcutGroup]?) {
        self.id = id
        self.appName = appName
        self.disEnable = disEnable
        self.order = order
        self.icon = icon
        self.version = version
        self.groups = groups
    }
}

// Convenience initializer for ShortcutItem so we can create modified copies (used for export)
extension ShortcutItem {
    init(id: UUID, action: String, disEnable: Bool?, keys: [String]?, steps: [ShortcutStep]?, description: String, context: String?, order: Int?) {
        self.id = id
        self.action = action
        self.disEnable = disEnable
        self.keys = keys
        self.steps = steps
        self.description = description
        self.context = context
        self.order = order
    }
}

// Wrapper used as a fallback if JSON is an object with an "apps" array
private struct AppsWrapper: Codable {
    let apps: [ShortcutApp]?
}

// Decode helper that accepts [ShortcutApp], {"apps": [...]}, or single ShortcutApp
private func decodeApps(from data: Data, using decoder: JSONDecoder = JSONDecoder()) throws -> [ShortcutApp] {
    // 1) Try array
    if let arr = try? decoder.decode([ShortcutApp].self, from: data), arr.count > 0 {
        return arr
    }
    // 2) Try wrapper
    if let wrapper = try? decoder.decode(AppsWrapper.self, from: data), let apps = wrapper.apps, apps.count > 0 {
        return apps
    }
    // 3) Try single object
    if let single = try? decoder.decode(ShortcutApp.self, from: data) {
        return [single]
    }
    // If all fail, throw
    struct DecodeError: Error {}
    throw DecodeError()
}
