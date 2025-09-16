// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class A7 {
    var a: Int64 = 1
}
class B7 {
    var b: Int64 = 2
}

class Data7 {
    static var a = A7()
    var b = B7()
    var c = 3
    var d = 4
}

func test6() {
    var data = Data7()
    var a = Data7.a.a
    var b = data.b.
    let b2 = Data7().b.b
    let c = Data7().c
    let d = data.d
}
