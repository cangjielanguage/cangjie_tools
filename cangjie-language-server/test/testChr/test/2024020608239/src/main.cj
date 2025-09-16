// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Data5<T> {
    var c = 0
    static let d: String = "hello"
    let e: T
    Data5(x: T, let a!: Int64 = 1) {
        this.e = x
    }
    init(x: T) {
        this.a = 3
        this.e = x
    }
    func foo(): T {
        return this.e
    }
    func foo(x: Int32): String {
        this.a.toString()
    }
    static func bar<U>(x: U): U {
        return x
    }

    static func s_func(): Rune {
        'a'
    }
}

var e1 = Data5<Int32>(1, a: 2).c

func f5() {
    var t1 = Data5<Int64>(2, a: 8)
    t1.foo(1)
    t1.a
    Data5<Int64>.bar<Float64>(2.0)
}
