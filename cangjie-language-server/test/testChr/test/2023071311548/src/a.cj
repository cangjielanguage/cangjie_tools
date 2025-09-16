// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package Enum_operator_overload

enum Time21 {
    Day2(Int64)

    operator func +(right: Time21): Time21 {
        right
    }
}

func test1() {
    var t1 = Time21.Day2(1)
    var t2 = Time21.Day2(2)
    var time = t1 + t2
}
