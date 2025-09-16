// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.2022031808222

func 2g<T>(x:T):T {x}
var a1:Int32 =1
var 2res4 = a1+1|> g<Int32>
var res5 = true |> g<Bool>

interface I11 {}

open class C1 <: I11 {}

struct Rectangle1 {
    var width1: Int32 = 10
    var 2length1: Int32 = 20
    init(width1: Int32, length1: Int32) {
        this.2width1 = width1
        this.length1 = length1
    }
    init(side: Int32) {
        this.width1 = side
        this.length1 = side
    }
}

struct B1<X, Y> {
    2B1(
        x: Int64,
        y: X,
        v!: Int64 = 1,
        private var z!: Y
    ) {}
}

enum Foo111 {
    | A | B | C
    func f1() {
        f2(this)
    }
    static func 2f2(v: Foo111) {
        match (v) {
            case $A => 0
            case $B => 1
            case $C => 2
        }
    }
    prop item: Int64 {
        get() {
            f1()
        }
    }
}

struct Foo3 {
    var i = 0
}
extend Foo3 {
    mut func f() { // ok
        i += 1
    }
}

extend Int64 {
    prop 2i: Int64 {
        get() {
            0
        }
    }
}

public class 2C111 {}

type ty1 = C111
type ty2 = ty1
type ty3 = ty2

extend 2ty3 {}

/*中文*/ func fun11(a: Int64): Int64{0}

var ss = Point2D(aa3,fun12)
var ss1 = ss(1.0)
/*中文*/ type 2Point2D = (C111, () -> Unit) ->( (Float32) -> Int32 )
