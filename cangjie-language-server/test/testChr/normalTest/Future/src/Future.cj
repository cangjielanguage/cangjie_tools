// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class A {
    var num: Int64 = 0
    operator func [](index: Int64): Int64 {
        return this.num + index
    }
    operator func [](index: Int64, value: Int64): Unit {
        this.num = index + value
    }
}
func foo(): A {
    return A()
}
func bar(): String {
    return ""
}

// The return type of foo() is A.
let ff11: Future<A> = spawn {
    foo()
}
// The return type of foo() is String.
let future_a: Future<String> = spawn {
    bar()
}
let future_c: Future<String> = spawn {
    foo()
}
func main():Int64 {
    var future_d = future_a
    future_a.get()
    ff11.isDone()
    return 0
}
