import Foundation
import Combine
import SwiftUI

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

    // UI selection state for export functionality
    @Published var selectedAppIndex: Int = 0
    @Published var selectedGroupIndex: [UUID: Int?] = [:]
    @Published var expandedSet: Set<UUID> = []
    
    // MARK: - App ordering
    @Published var customAppOrder: [String] = [] {
        didSet {
            UserDefaults.standard.set(customAppOrder, forKey: "CustomAppOrder")
            updateFilteredApps()
        }
    }
    
    // MARK: - App visibility
    @Published var hiddenApps: Set<String> = [] {
        didSet {
            UserDefaults.standard.set(Array(hiddenApps), forKey: "HiddenApps")
            updateFilteredApps()
        }
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
        // Load custom app order from UserDefaults
        if let savedOrder = UserDefaults.standard.stringArray(forKey: "CustomAppOrder") {
            customAppOrder = savedOrder
        }
        
        // Load hidden apps from UserDefaults
        if let savedHiddenApps = UserDefaults.standard.stringArray(forKey: "HiddenApps") {
            hiddenApps = Set(savedHiddenApps)
        }
        
        // discover bundled shortcutJsons
        scanShortcutJsons()

        // Auto-load all available JSON files
        if !availableJsons.isEmpty {
            self.selectedJsonFiles = Set(availableJsons.map { $0.fileName })
            loadSelectedFiles()
        } else {
            // Fallback to default single file if no JSONs found
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

        // Debug: print bundle resourceURL and its top-level children to help diagnose missing folders
        if let res = Bundle.main.resourceURL {
            print("[ShortcutStore] bundle.resourceURL = \(res.path)")
            let fm = FileManager.default
            if let children = try? fm.contentsOfDirectory(atPath: res.path) {
                print("[ShortcutStore] bundle top-level children: \(children)")
            } else {
                print("[ShortcutStore] failed to list bundle resourceURL children")
            }
        } else {
            print("[ShortcutStore] Bundle.resourceURL is nil")
        }
        

        var entries: [JsonFileEntry] = []
        let decoder = JSONDecoder()
        var seenNames: Set<String> = []

        // First, try to get files specifically in shortcutJsons subdirectory (preferred)
        // Choose subdirectory based on current preferred language. If English, prefer shortcutJsons_en.
        // Decide preferred subdirectory based on the device/user language preference.
        // Use Locale.preferredLanguages so that the user's language setting controls which folder is used.
        let preferredSubdir: String
        // Use device/user preferred language to decide folder. Do NOT rely on Bundle.preferredLocalizations
        // so Xcode Run As/App Language overrides won't change which folder we pick.
        let userLang = Locale.preferredLanguages.first ?? ""
        if userLang.hasPrefix("en") {
            preferredSubdir = "shortcutJsons_en"
        } else {
            preferredSubdir = "shortcutJsons"
        }

        if let subUrls = Bundle.main.urls(forResourcesWithExtension: "json", subdirectory: preferredSubdir), subUrls.count > 0 {
            print("[ShortcutStore] found \(subUrls.count) files in \(preferredSubdir) subdirectory")
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

        // If Bundle API didn't return files for the preferred subdir, try a case-insensitive
        // match of the directory name under the bundle resource URL and enumerate its files.
        if let resourceURL = Bundle.main.resourceURL {
            let fm = FileManager.default
            if let children = try? fm.contentsOfDirectory(at: resourceURL, includingPropertiesForKeys: nil, options: []) {
                if let matchDir = children.first(where: { $0.hasDirectoryPath && $0.lastPathComponent.lowercased() == preferredSubdir.lowercased() }) {
                    print("[ShortcutStore] found directory (case-insensitive match): \(matchDir.path)")
                    if let jsonFiles = try? fm.contentsOfDirectory(at: matchDir, includingPropertiesForKeys: nil, options: []).filter({ $0.pathExtension.lowercased() == "json" }), jsonFiles.count > 0 {
                        for url in jsonFiles {
                            let fname = url.lastPathComponent
                            guard !seenNames.contains(fname) else { continue }
                            seenNames.insert(fname)

                            print("[ShortcutStore] inspecting matched-dir file: \(url.path)")
                            do {
                                let data = try Data(contentsOf: url)
                                do {
                                    let apps = try decodeApps(from: data, using: decoder)
                                    let display = apps.first?.appName ?? fname
                                    entries.append(JsonFileEntry(fileName: fname, displayName: display, url: url))
                                    print("[ShortcutStore] decoded JSON from matched-dir: \(fname) -> display=\(display)")
                                } catch {
                                    print("[ShortcutStore] decode error for matched-dir file \(fname): \(error)")
                                }
                            } catch {
                                print("[ShortcutStore] failed to read data at url: \(url.path) error: \(error)")
                            }
                        }

                        // Deduplicate as before
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
                        print("[ShortcutStore] scan complete (matched-dir). found \(availableJsons.count) json entries (deduped)")
                        return
                    }
                }
            }

            // If the preferred subdir isn't present in the bundle, some build setups may have
            // placed the JSON files at the bundle root. As a pragmatic fallback, use top-level
            // JSONs filtered by locale: English => use *_en.json only; others => exclude *_en.json.
            if let topJsons = try? fm.contentsOfDirectory(at: resourceURL, includingPropertiesForKeys: nil, options: []).filter({ $0.pathExtension.lowercased() == "json" }), topJsons.count > 0 {
                let wantEnglish = preferredSubdir.hasSuffix("_en")
                let filteredTop = topJsons.filter { url in
                    let name = url.lastPathComponent
                    if name == "Shortcuts.json" { return false }
                    if wantEnglish {
                        return name.lowercased().hasSuffix("_en.json")
                    } else {
                        return !name.lowercased().hasSuffix("_en.json")
                    }
                }
                if filteredTop.count > 0 {
                    print("[ShortcutStore] found top-level JSONs matching locale (count=\(filteredTop.count)). Using them as fallback")
                    for url in filteredTop {
                        let fname = url.lastPathComponent
                        guard !seenNames.contains(fname) else { continue }
                        seenNames.insert(fname)

                        print("[ShortcutStore] inspecting top-level file: \(url.path)")
                        do {
                            let data = try Data(contentsOf: url)
                            do {
                                let apps = try decodeApps(from: data, using: decoder)
                                let display = apps.first?.appName ?? fname
                                entries.append(JsonFileEntry(fileName: fname, displayName: display, url: url))
                                print("[ShortcutStore] decoded JSON from top-level: \(fname) -> display=\(display)")
                            } catch {
                                print("[ShortcutStore] decode error for top-level file \(fname): \(error)")
                            }
                        } catch {
                            print("[ShortcutStore] failed to read data at url: \(url.path) error: \(error)")
                        }
                    }

                    var uniqueByKeyTop: [String: JsonFileEntry] = [:]
                    for e in entries {
                        let key = e.displayName.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
                        if uniqueByKeyTop[key] == nil {
                            uniqueByKeyTop[key] = e
                        } else {
                            print("[ShortcutStore] skipping duplicate displayName (top-level): \(e.displayName) at \(e.url.path)")
                        }
                    }
                    availableJsons = uniqueByKeyTop.values.sorted { $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending }
                    print("[ShortcutStore] scan complete (top-level). found \(availableJsons.count) json entries (deduped)")
                    return
                }
            }
         }

        // Per-locale policy: nothing found for preferred subdir or top-level fallback
        print("[ShortcutStore] no JSON files found in \(preferredSubdir). availableJsons set to empty per locale policy")
        availableJsons = []
        return
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
            
            // Filter user-hidden apps unless showHidden is true
            if hiddenApps.contains(app.appName) && !showHidden {
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
        
        // Apply custom app order if exists
        if !customAppOrder.isEmpty {
            filteredApps = applyCustomOrder(filteredApps)
        }

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
        
        // Initialize default group selection for apps that don't have selection yet
        initializeDefaultGroupSelection()
    }
    
    // Initialize default group selection (first group for each app)
    private func initializeDefaultGroupSelection() {
        for app in filteredApps {
            if selectedGroupIndex[app.id] == nil, let groups = app.groups, groups.count > 0 {
                selectedGroupIndex[app.id] = 0 // Select first group by default
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
    
    // MARK: - App ordering methods
    private func applyCustomOrder(_ apps: [ShortcutApp]) -> [ShortcutApp] {
        var orderedApps: [ShortcutApp] = []
        var remainingApps = apps
        
        // First, add apps in the custom order
        for appName in customAppOrder {
            if let index = remainingApps.firstIndex(where: { $0.appName == appName }) {
                orderedApps.append(remainingApps.remove(at: index))
            }
        }
        
        // Then add any remaining apps that weren't in the custom order
        orderedApps.append(contentsOf: remainingApps)
        
        return orderedApps
    }
    
    // Method to move an app in the custom order
    func moveApp(from source: IndexSet, to destination: Int) {
        let appNames = filteredApps.map { $0.appName }
        var newOrder = customAppOrder.isEmpty ? appNames : customAppOrder
        
        // Ensure all current apps are in the order array
        for appName in appNames {
            if !newOrder.contains(appName) {
                newOrder.append(appName)
            }
        }
        
        newOrder.move(fromOffsets: source, toOffset: destination)
        customAppOrder = newOrder
    }
    
    // MARK: - App visibility methods
    func toggleAppVisibility(_ appName: String) {
        if hiddenApps.contains(appName) {
            hiddenApps.remove(appName)
        } else {
            hiddenApps.insert(appName)
        }
    }
    
    func getAllAvailableApps() -> [ShortcutApp] {
        // Return all apps from loaded JSON files, regardless of hiddenApps filter
        // This is used for the settings UI to show all apps with visibility toggles
        return apps.compactMap { (app) -> ShortcutApp? in
            // Only filter disabled apps from JSON
            if app.disEnable == true {
                return nil
            }
            return app
        }.sorted { lhs, rhs in
            // Apply custom order if exists, otherwise use JSON order
            if !customAppOrder.isEmpty {
                let lhsIndex = customAppOrder.firstIndex(of: lhs.appName) ?? Int.max
                let rhsIndex = customAppOrder.firstIndex(of: rhs.appName) ?? Int.max
                return lhsIndex < rhsIndex
            } else {
                return (lhs.order ?? 0) < (rhs.order ?? 0)
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

// MARK: - Export Models
struct ExportGroupEntry: Codable {
    let appName: String
    let disEnable: Bool?
    let order: Int?
    let icon: String?
    let version: String?
    let group: ShortcutGroup
    
    private enum CodingKeys: String, CodingKey {
        case appName, disEnable, order, icon, version, group
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
