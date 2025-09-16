// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.variablePattern
func testVariablePattern() {
    let score: Int64 = 90
    let PASS = 60
    let Full1 = 100

    /**
    * this is good
    */
    let good = 70
    var scoreResult: String = match (score) {
        case PASS => "pass" // constant pattern.
        case Full1 => "Full" // ok, but not recommend.
        case good => "good" /* matched. warning: 'good' has been defined outside the match expression,
                            are you sure using 'good' as a variable pattern here ?*/
    }
    func test() {
        let score: Int64 = 90
        var scoreResult: Int64 = match (score) {
        case 60 => 60
        case 70 | 80 => 80
        case 90 | 100 => 100 // matched
        case failScore => // variable pattern
        let passNeed = 60 - failScore
        passNeed
        }
    }
}
