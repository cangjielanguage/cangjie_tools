// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.wildcardPattern
func testWildcardPattern() {
    let score: Int64 = 90
    var scoreResult: String = match (score) {
    case 60 => "pass"
    case 70 | 80 => "good"
    case 90 | 100 => "excellent" // matched
    case _ => "fail" // wildcard pattern: used for default case
    }
}
