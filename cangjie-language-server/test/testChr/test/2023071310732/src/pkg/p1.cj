// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg

public class AB {
    public init(x: Int32, y: Int32) {
        this.a = x
        this.c = y
    }
    public AB(b: Int64, public let a: Int32, var c: Int32) {}

    public let num = 1
}


func t2() {
    var m1 = AB(2, 8)
    var m2 = AB(1, 1, 6)
    let cc = m2.a

}

