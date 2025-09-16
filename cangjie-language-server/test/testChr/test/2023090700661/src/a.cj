// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface Create<T> {
    static prop create: T
}
class Test <: Create<Test> {
    func a() {
        println("a")
    }
    public static prop create: Test {
        get() {
            return Test()
        }
    }
}
open class A<T> where T <: Create<T> {
    prop test: T {
        get() {
            return T.create
        }
    }
}

class B <: A<Test> {}

main(): Unit {
    B().test.a
}
