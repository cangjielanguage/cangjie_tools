// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func f1(a: Int32, b: Int32, c!: Int32 = 3, d!: Int32 = 4): Int32 {
    a + b + c + d
}
func test() {
    f1(1, 2) // 10, f1(1, 2, 3, 4)
    f1(1, 2, c: 5) // 12, f1(1, 2, 5, 4)
}

var x = 10
var y = 10
func g() {}
var y1 = g()
func f2(a!: Int64 = x * 2 + y, b!: () -> Unit = g) {}
class AAA {
    static private var x = 10
    func f(a!: Int64 = x) { // OK, public method can use private static field.
        print("${a}")
        x = x + 10
        print("${x}")
    }
}

