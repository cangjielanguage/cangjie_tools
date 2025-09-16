// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class D {
    var num: Int64 = 0
    operator func ^^(x: D, index: Int64, value: Int64): Unit {
        x.num = index + value
    }
    operator func +(x: D, index: Int64, value: Int64): Unit {
        x.num = index + value
    }
}
