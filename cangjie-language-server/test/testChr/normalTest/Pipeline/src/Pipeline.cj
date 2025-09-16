// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func f(x: Int32): Int32 { x + 1 }

let a: Int32 = 1
var res = a |> f // ok
var res1 = a |> {x: Int32 => x + 1}// ok

func h(b: Bool) { b }
let res3 = a < 0 || a > 10 |> h // Equivalence：(a < 0 || a > 10) |> h

func g<T>(x: T): T { x }
var res4 = a |> g<Int32> // ok

class A{
    let a: Int32
    let b: Int32
    init(x: Int32) {
        a = x
        b = 0
    }
    init(x: Int32, y: Int32) {
        x |> init // error: `init` is not a valid expression
        b = y
    }
}

// PIPELINE with `asFunc`
class A1 {
    func asFunc(x: Int32) {
        x
    }
}
let obj = A1()
let a1: Int32 = 1
let res5 = a1 |> obj // Equivalence：obj.asFunc(a)
