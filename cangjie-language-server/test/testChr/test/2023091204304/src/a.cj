// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

struct CCC<T, U> {
    var a: T
    var b: U
    init(a: T, b: U) {
        this.a = a
        this.b = b
    }
}

var myC = CCC(1, "b: U")
var myC1 = CCC<Int32, Int64>(3, 4)
