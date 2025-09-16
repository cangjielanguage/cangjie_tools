// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I6 {
    func f1(): Unit
    static func f2(): Unit
}

interface I6_2 <: I6 {
    override func f1(): Unit {}
}

struct SS1 <: I6_2 {
    public static func f2(): Unit {}
    func f3() {}
}

func test10() {
    var b2 = SS1.f2()
    var b3 = SS1().f3()
    var b1 = SS1().f1()
}
