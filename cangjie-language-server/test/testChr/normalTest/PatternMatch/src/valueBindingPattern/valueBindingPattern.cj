// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.valueBindingPattern
func testvalueBindingPattern() {
     let scoreTuple = (("Allen", 90), ("Jack", 60))
     var scoreResult = match (scoreTuple) {
     case ((_, _), secondStudent@ (_, _)) => (secondStudent[1]+ secondStudent[1])
     }
 }
