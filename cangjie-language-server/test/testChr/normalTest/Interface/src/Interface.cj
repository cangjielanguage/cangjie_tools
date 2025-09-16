// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface ERRI1

public interface I {}

interface I1 {
	func f(): Unit
}
interface I2 <: I1 {}

interface MyInterface {
	func f1(): Unit // Default implementation not included
	static func f2(): Unit // Default implementation not included
	func f3(): Unit { // Default implementation included
		return
	}
	static func f4(): Unit { // Default implementation included
		return
	}
}

interface MyInterface2 {
    let a = 1
	func f1(a!: Int64): Unit // OK
	func f2(a!: Int64 = 1): Unit // Error, cannot have parameter default
}

interface MyInterface3 {
    public func f1(): Unit // Access modifiers cannot be used
    private static func f2(): Unit // Access modifiers cannot be used
}

interface I5 {
    static func f1(): Unit
    static func f2(): Unit {}
}
let xI = I1.f() // Error, cannot directly call

interface I6 {
    func f1(): Unit {}
    static func f2(): Unit {}
}

interface I7 <: I1 & I999 {
    override func f1(): Unit {} // Error
    override static func f2(): Unit {} // Error
}
