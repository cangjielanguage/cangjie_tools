// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


// case 1
interface MyInterface {
    func f1(): Int8 // Default implementation not included
    static func f2(): Unit // Default implementation not included
    func f3(): Unit { // Default implementation included
        return
    }
    static func f4(): Unit { // Default implementation included
        return
    }
    func f6(a: Int8): Int8 { // Default implementation included
            return 1
        }
}


interface I1 {
    let a = 1
    func f1(a: Int8):Unit
    static func f3(): Int16{return 3}
}
interface I2 {
    func f2(): Unit {}
}
open class A {
    open func f(): Unit {} // ok
}
class B <: A & I1 & I2{
    func f2(): Unit{}
    func f1(a: Int8): Unit{return }
    override func f(): Unit {} // ok
}

let classB_A = B()
let classA_A = A()
let test01 = classA_A.f()
let test02 = classA_A.f()
let te = B()

let a = te.f()

interface II1 {
	 func  ff1():Unit{}
}
class CC <: II1{
	 func ff1(): Unit{
	    return
    }
}
let ss:II1 = CC()
let aa = ss.ff1()


