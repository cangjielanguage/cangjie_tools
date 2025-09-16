// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import c
class C{
    var a:Int64
    var b:Float64

    init(a:Int64,b:Float64) {
        this.a = a
        this.b = b
    }

    init(a:Int64) {
        this.a = a
        b = 1.1
    }
}
// 2021111024801
var a = C(1).
var b = C(1, 2.2).

type B1 = BB


class BB {
    static var b:Int16 = 1
    static func foo(){}
    var cc = 1
}
let ccc = BB()

func test8(){
    // 2021111115617
    B1.
}
