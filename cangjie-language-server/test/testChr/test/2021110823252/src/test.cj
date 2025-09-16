// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

var a = 1


// 泛型类名称+泛型成员变量+主构造成员函数+成员函数重载语法高亮
class Data<T> {
    var c = 0
    static let d: String = "hello"
    let e: T
    Data(x:T, let a!: Int64 =1) {
        this.e = x
    }
    init(x: T){
        this.a = 3
        this.e = x
    }
    func foo(): T {
        return this.e
    }
    func foo(x: Int32): String {
        this.a.toString()
    }
    static func bar<U>(x:U): U {
        return x
    }

    static func s_func(): Rune { 'a' }
}

func run(): Int64 {
    return Data<Int32>(1, a:2).c
}

func main(): Int64 {
    return run()
}

