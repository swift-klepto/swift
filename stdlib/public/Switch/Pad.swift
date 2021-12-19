//===----------------------------------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2021 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

// MARK: Convenience methods

/// Convenience method for using padConfigureInput with HidNpadStyleTag
public func padConfigureInput(_ max_players: UInt32, _ style_set: HidNpadStyleTag) {
    padConfigureInput(max_players, style_set.rawValue)
}

/// Convenience extension to use & operator between UInt64 and HidNpadButton directly
public extension UInt64 {
    static func & (left: UInt64, right: HidNpadButton) -> Bool {
        return left & UInt64(right.rawValue) != 0
    }
}

// MARK: Swift API
