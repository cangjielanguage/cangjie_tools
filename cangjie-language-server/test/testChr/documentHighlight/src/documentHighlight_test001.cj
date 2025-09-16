// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func arith_test(): int64 {
    var a: int16 = 127 + 128
    var b: int16 = 127 - 128
    var c: int16 = 128 / 16
    var d: int16 = 128 % 12
    var e: int16 = 127 * 128
    var a1: int16 = 2 ** 10
    var a2: int16 = +8
    var a3: int16 = -8
    var a4:int16 = ~10
    var a5:int16 = 10 << 1
    var a6:int16 = 10 >> 1
    var a7:int16 = 10 & 15
    var a8:int16 = 10 ^ 15
    var a9:int16 = 10 | 15
    
    //let f = 1.2/0
    //if f != infinity {
    //    return 1
    //}
    //let g = -1.2/0
    //if g != -infinity {
    //   return 1
    //}
    
    //let h = 0/0.0
    //if h != NaN {
    //   return 1
    //}

    return 0
}
