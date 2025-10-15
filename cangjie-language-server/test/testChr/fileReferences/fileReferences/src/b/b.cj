// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.b

public open class B {
	let x = 1
    static let height = 20
    public B() {
    }
}

public struct Rectangle1 {
    let width: Int64
    let height: Int64

    public init(width:Int64, height:Int64) {
        this.width = width
        this.height = height
    }

    public Rectangle1(width:Int64) {
        this.width = width
        this.height = width
    }
}

public open interface I2 { // 'open' modifier is optional.
}

public type I8 = Int64

public func test4() {}

public let s2 = "1"

public enum Color2 {
	| RED | GREEN | BLUE
}