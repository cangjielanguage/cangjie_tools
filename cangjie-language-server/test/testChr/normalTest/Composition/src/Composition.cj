// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func composition_a(x: Int32): Float32 { Float32(x) }
func composition_b(x: Float32): Int32 { Int32(x) }

var fg = composition_a ~> composition_b
let lambdaComp = ({x: Int32 => x}) ~> composition_a // ok
func h1<T>(x: T): T { x }
func h2<T>(x: T): T { x }
var hh = h1<Int32> ~> h2<Int32>

class A1{
    func asFunc(x: Int32): Int32 {
        x
    }
    init() {
        var fg1 = A1() ~> init // error: `init` is not a valid expression
    }
}
class B1{
    func asFunc(x: Float32): Float32 {
        x
    }
}
let objA = A1()
let objB1 = B1()
let af = objA ~> composition_a // ok
let fb = composition_a ~> objB1 // ok
let aa1 = objA ~> objA
