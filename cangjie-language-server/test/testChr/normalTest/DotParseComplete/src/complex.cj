// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class A {
    func b() : Int64 { 0 }
    func b(v!: Int64 = 1) : B { B() }
}
class B { var c = 1 }
func a(x: Rune) : Unit {}
func a(v: Int64) : A { A() }

func test() {
    a(1).b(v: 2).
}
