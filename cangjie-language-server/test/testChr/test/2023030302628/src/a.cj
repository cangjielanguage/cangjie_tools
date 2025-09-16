// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class A_6 {
    func test6(x: Float64) {
        return 1
    }
    func test6_1(x: Int64) {
        return 1.1
    }
}
let r1 = A_6()
var r2 = r1.test6 ~> r1.test6_1
var r3 = r1.
