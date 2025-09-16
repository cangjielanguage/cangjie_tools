// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// Catch with exceptionTypePattern.
let arrayTest: RefArray<Int32> = RefArray<Int32>([0, 1, 2])
class A<:Throwable {}
func test0001( )  {
	 try {
         throw A()
         let temp = arrayTest[0] + 1 // Will never be executed.
     } catch (e: ArithmeticException) {
         print("an arithmeticException happened: ")
     } finally {
         print("the finally block is executed")
     }
}

class B{}
// Caught by catchE().
func catchAE() {
    let arrayTest: RefArray<Int32> = RefArray<Int32>([0, 1, 2])
    try {
        throwAE() // caught by catchE()
    } catch (e: A) {
        print("an ArrayIndexOutOfBoundsException happened: ")
    }
}

// Terminate function is executed.
func notCatchAE() {
    let arrayTest: RefArray<Int32> = RefArray<Int32>([0, 1, 2])
    throwE()
}

// func throwE()
func throwAE() {
    throw A()
}
func throwE() {
    throw B()
}
