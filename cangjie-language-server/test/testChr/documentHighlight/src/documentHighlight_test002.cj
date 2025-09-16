// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func documentHighlight_test002(): int64 {
    var a: int16 = 127 + 128
    var b: int16 = 127 - 128
    var c: int16 = 128 / 16
    var d: int16 = 128 % 12
    var e: int16 = 127 * 128
    let a1: int16 = 2 ** 10
    let a2: int16 = +8
    let a3: int16 = -8
    let a4:int16 = ~10
    let a5:int16 = 10 << 1
    let a6:int16 = 10 >> 1
    let a7:int16 = 10 & 15
    let a8:int16 = 10 ^ 15
    let a9:int16 = 10 | 15
    func test(){
        var inaa:int16 = a8
    }
    func test1(){
        test()
        var inaa:int16 = a1
    }
    return 0
}
