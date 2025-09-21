import SwiftUI
import UIKit

struct ContentView: View {
    @StateObject private var store = ShortcutStore()

    var body: some View {
        NavigationView {
            List {
                ForEach(store.apps) { app in
                    Section(header: Text(app.appName)) {
                        if let groups = app.groups {
                            ForEach(groups) { group in
                                DisclosureGroup(group.groupName) {
                                    if let shortcuts = group.shortcuts {
                                        ForEach(shortcuts) { item in
                                            HStack {
                                                VStack(alignment: .leading) {
                                                    Text(item.action ?? "-")
                                                        .font(.headline)
                                                    if let desc = item.description {
                                                        Text(desc)
                                                            .font(.caption)
                                                            .foregroundColor(.secondary)
                                                    }
                                                }
                                                Spacer()
                                                if let keys = item.keys {
                                                    Button(action: {
                                                        UIPasteboard.general.string = keys.joined(separator: " + ")
                                                    }) {
                                                        Text(keys.joined(separator: " + "))
                                                            .font(.caption)
                                                            .padding(6)
                                                            .background(Color.gray.opacity(0.15))
                                                            .cornerRadius(6)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            .navigationTitle("EasyShortcutKey")
        }
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
