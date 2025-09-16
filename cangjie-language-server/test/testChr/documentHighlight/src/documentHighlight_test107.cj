// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func multiValues(a: Int32, b: Int32) { (a + b, a - b) }

func test_tuple1() = {
    let tuplePIE = (3.14, "PIE")
    var pointOne = (2.4, 3.5)
    var pointTwo1 = (2, 3, 4)
    var pointThree1 = (2, 3, 4)
    var (x, y) = multiValues(8,24)
    var z = multiValues(8,24)
    pointTwo1 == pointThree1
}
