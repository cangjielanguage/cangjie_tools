// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class TestA{
    let TestA_intA = 64
    func TestA_func(){
        return TestA_intA
    }
}
open class TestAA{
     init(a: Int32)  {
 	 TestA_intA = 555
     }

    let TestA_intA = 64
    func TestA_func(){
        return TestA_intA
    }
}
class TestBB <: TestA {
    let TestB_intB = T
    init(a: Int32)  {
	 TestB_intB = 555
    }

    func TestB_func(){
	       return TestA_intA
    }
}
class TestB <: TestA{
    let TestB_intB = 5
    init() {
         TestA_intA = 555
     }
    init(a: Int32)  {
	 TestB_intB = 555
    }

    func TestB_func(){
	       return TestA_intA
    }
}
let classA = TestA()
let testAB = classA.TestA_intA
let testAC = classA.TestA_func()


let classB = TestB()
let testBB = classB.TestA_intA
let testBC = classB.TestA_func()



