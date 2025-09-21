import Foundation
import Combine

struct ShortcutApp: Codable, Identifiable {
    let id = UUID()
    let appName: String
    let order: Int?
    let icon: String?
    let version: String?
    let groups: [ShortcutGroup]?
}

struct ShortcutGroup: Codable, Identifiable {
    let id = UUID()
    let groupName: String
    let order: Int?
    let description: String?
    let shortcuts: [ShortcutItem]?
}

struct ShortcutItem: Codable, Identifiable {
    let id = UUID()
    let action: String?
    let keys: [String]?
    let description: String?
    let order: Int?
}

class ShortcutStore: ObservableObject {
    @Published var apps: [ShortcutApp] = []

    init(filename: String = "Shortcuts.json") {
        load(from: filename)
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
            }
        } catch {
            print("Failed to load/parse shortcuts: \(error)")
        }
    }
}
