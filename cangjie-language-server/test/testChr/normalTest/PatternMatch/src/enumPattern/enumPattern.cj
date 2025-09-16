// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.enumPattern
enum TimeUnit {
| Year
| Year(Int64)
| Month(Int64, Int64)
| Day(Int64, Int64, Int64)
| Hour(Int64, Int64, Int64, Int64)
}
func testEnumPattern() {
    let oneYeara = TimeUnit.Year(100000)
    let oneYear = TimeUnit.Year(100000)
    var howManyHours:Int64 = match (oneYear) {
    case Year => 365
    case Year(y) => y * 365 * 24 // matched
    case Month(y, m) => y * 365 * 24 + m * 30 * 24
    case Day(y, m, d) => y * 365 * 24 + m * 30 * 24 + d * 24
    case Hour(y, m, d, h) => y * 365 * 24 + m * 30 * 24 + d * 24 + h
    }
    let twoYear = TimeUnit.Year(2)
    var howManyYears: Int64 = match (twoYear) {
    case Year(y) | Month(y, m) => y // error: different number of variables are introduced
    case Year(y) | Month(x, _) => y // error: different name of variables are introduced
    case Year(y) => y // ok

    }

}
