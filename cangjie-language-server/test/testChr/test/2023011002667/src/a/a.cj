// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package a

class Ra1 {
    Ra1(var a!: Int64, var b!: Float64) {
        this.a = a
        this.b = b
    }
    init(a1: Int64) {
        this.a = a1
        b = 1.1
    }
}

func test25() {
    var a = Ra1(a: 1, b: 2.2).a
    var b = Ra1(1).b
}


