// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.a

public open class testclass {
	let x = 1
    static let height = 20
    public testclass() {
    }
}

public struct Rectangle {
    let width: Int64
    let height: Int64

    public init(width:Int64, height:Int64) {
        this.width = width
        this.height = height
    }

    public Rectangle(width:Int64) {
        this.width = width
        this.height = width
    }
}

public open interface I1 { // 'open' modifier is optional.
}

public type I64 = Int64

func test1() {}

public let s = "1"

enum Color {
	| RED | GREEN | BLUE
}

