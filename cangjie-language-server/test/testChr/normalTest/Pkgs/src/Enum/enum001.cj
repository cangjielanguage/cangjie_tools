// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package Enum

// enum的普通枚举值和函数形式的枚举值
enum TimeUnit {
    Year | Month | Day(Int32) | Hour(Int64)
}

enum TimeUnit1 {
    Year | Month | Day(Int32) | Hour(Int64)
}
func test1() {
    var time = TimeUnit.Day(3)
    var time2 = TimeUnit1.Hour(23)
    var res: Int32 = match (time) {
        case TimeUnit.Year => 0
        case Month => 1
        case Day(x) => 2
        case Hour(y) => 3

    }
    var res2: Int32 = match (time2) {
        case Year => 0
        case Month => 1
        case Day(x) => 2
        case Hour(y) => 3
    }
}

