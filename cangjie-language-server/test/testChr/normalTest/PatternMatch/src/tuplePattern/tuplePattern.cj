// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.tuplePattern
func testTuplePattern() {
    let scoreTuple = ("Allen", 90)
    var scoreResult = match (scoreTuple) {
    case ("Bob", 90) => 90
    case ("Allen", score) => score // matched
    case (_, 100) => 100
    case (_, _) => 0
    }

    var scoreResult2:String = match (scoreTuple) {
    case ("Bob", 90) => "90"
    case ("Allen", score) => "Allen" // matched
    case (_, 100) => "100"
    case (_, _) => "All"
    }
}
