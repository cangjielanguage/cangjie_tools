// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func testUnit4() {
    let x = -10
    let y = match (x) {
        case 0 => "zero"
        case n =>
            let n = 0
            println(n)
            "x is not zero"
    }
}

func testUnit5(b: Bool) {
    var a = 1
    if (b) {
        var a = 3
    } else {
        var a = 2
    }
}

func testUnit6() {
    var x = -10
    var b: Bool
    let y1 = Array<Int64>()
    try {
        let y = Array<Int64>()
        b = y.contains(x)
    } catch (name: ArithmeticException) {
        let z = 1
    } finally {
        x = 0
    }
}
