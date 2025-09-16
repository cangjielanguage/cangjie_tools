// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.sequencePattern

//不支持标准库里的Array
 let a = @{50, 70, 90} // sorted array
 //var ss = "empty score list"
 var r1: Int64 = match (a) {
 case [] => 0
 case [50, ...] => 50 // matched
 case [..., 90] => 90
 case [low, ..., high] => high - low
 case [...] => 60
 }

 let b: List<Int64> = [60, 80, 100] // sorted list
 var r2:Unit = match (b) {
 case [] => var ss = 0
 case [0, ...] => var ss = 0
 case [..., 99] => var ss = 99
 case [low, ..., high] => var ss =high // matched
 case [...] => var ss = "default case"
 }
 let c: List<Int64> = [40, 60, 80, 100] // sorted list
 var r3: Int64 = match (c) {
 case [] => 0
 case [a1, a2, a3, ..., b1, b2, b3] => (a1 + a2 + a3 + b1 + b2 + b3) // matched
 case [...] => 100
 }

