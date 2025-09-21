import SwiftUI
import SwiftData

@main
struct EasyShortcutKeyApp: App {
    var body: some Scene {
        WindowGroup {
            ContentView()
                .modelContainer(for: [HiddenShortcut.self])
        }
    }
}
