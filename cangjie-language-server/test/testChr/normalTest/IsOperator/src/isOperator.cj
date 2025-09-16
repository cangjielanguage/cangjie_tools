// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// e is T 是一个用于类型检查的表达式，e is T 的类型是Bool。其中e 可以是任何类型的表达式，T 可
// 以是任何类型。
// 当e 的运行时类型R 是T 的子类型时，e is T 的值为true，否则值为false。
open class Base {
    var name: String = "Alice"
}
class Derived1 <: Base {
    var age: UInt8 = 18
}
class Derived2 <: Base {
    var gender: String = "female"
}
func test(): Int64 {
    var testVT = 1 is Int64 // testVT = true
    testVT = 1 is String // testVT = false
    testVT = true is Int64 // testVT = false
    testVT = [1, 2, 3] is List<Int64> // testVT = true
    /**
    * this is base1
    */
    let base1_Rename: Base = Base()
    let base2: Base = Derived1()
    let base3: Base = Derived2()
    let derived1: Derived1 = Derived1()
    let derived2: Derived2 = Derived2()
    var test = base1_Rename is Base // test = true
    test = base1_Rename is Derived1 // test = false
    test = base1_Rename is Derived2 // test = false
    test = base2 is Base // test = true
    test = base2 is Derived1 // test = true
    test = base2 is Derived2 // test = false
    test = base3 is Base // test = true
    test = base3 is Derived1 // test = false
    test = base3 is Derived2 // test = true
    test = derived1 is Base // test = true
    test = derived1 is Derived1 // test = true
    test = derived1 is Derived2 // test = false
    test = derived2 is Base // test = true
    test = derived2 is Derived1 // test = false
    test = derived2 is Derived2 // test = true
    let riss = Base() as Base
    return 0
}
