// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.inPattern
func testInPattern() {
    let scorePass: Set<Int64> = #{60, 70, 80, 90, 100}
    let scoreLow: List<Int64> = [40, 50]
    let scoreLowsssss = ""
    let score = 500
    var scoreResult: String = match (score) {
    case in $scorePass => "pass"
    case in $scoreLow => ""
    case in [10, 20, 30] => "very low score"
    case _ => "low score" // matched
    }
    //支持list,set,map,不支持refArray
}
