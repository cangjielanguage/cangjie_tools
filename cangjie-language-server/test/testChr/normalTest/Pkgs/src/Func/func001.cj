// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package Func

func foo(x: Int64, y!: Int64, z!: Int64 = 3): Int64 {
    return x + y + z
}

func foo(x: Int64, y: (Int64) -> Int64): Int64 {
    return y(x)
}

func foo(): String {
    return "Nothing"
}

func maain(): Int64 {
    var a = 1
    var b = 2
    var bar = {x: Int64 => x}
    foo(a, bar)
    foo(a, y: b)
    foo(a, y: b, z: 4)

    return 0
}

