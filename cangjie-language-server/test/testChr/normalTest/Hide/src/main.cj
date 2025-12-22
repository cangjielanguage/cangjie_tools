// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos

class A <: AAA {
    func test() {
    
    }
}

class B <: BBB {
    func test() {
        
    }
}

class C<T> <: T2<T> {
    func test() {
        
    }
}

main(): Unit {
    var a = A()
    var b = B()
    var c = C<String>()
    var e = EEE.A
    var cc = CCC(1, 2)
    a
    b
    c
    e
    cc
    
}