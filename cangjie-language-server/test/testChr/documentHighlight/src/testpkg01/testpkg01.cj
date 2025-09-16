// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testpkg01

public let one : int32 = 1
public let two : int32 = 2

public struct Test{
    let width: int32 = 2
    let length: int32 = 5
    init(a:int32, b:int32) {
        width = a
        length = b   
    }

    func area(): int32 {
        return width * length
    }
}

public enum TimeUnit {
    Year | Month | Day | Hour
}

public func testpkgFunc(): int32 {
    return 1
}
