import Foundation
import SwiftData

@Model
final class HiddenShortcut {
    @Attribute(.unique) var id: UUID

    init(id: UUID) {
        self.id = id
    }
}
