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

/// R_SUCCEEDED macro reimplementation
public func R_SUCCEEDED(_ result: Result) -> Bool {
    return result == 0
}

/// R_FAILED macro reimplementation
public func R_FAILED(_ result: Result) -> Bool {
    return result != 0
}

// MARK: Swift API

/// Convenience extension to add .succeeded() .failed() methods to Result
public extension Result {
    /// Returns true if the Result succeeded
    func succeeded() -> Bool {
        return R_SUCCEEDED(self)
    }

    /// Returns false if the result succeeded
    func failed() -> Bool {
        return R_FAILED(self)
    }
}
