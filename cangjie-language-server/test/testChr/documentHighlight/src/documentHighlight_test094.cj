// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

trait Num941 {
    func plus94(a: This, b: This): This
}

trait Num941 {
    func plus94(a: This, b: This): This
}

trait Num942 {
    func self94(a: This): This
}

append int32 with Num941 ,Num942 {
    func plus94(a: int32, b: int32): int32 { a + b }
    func self94(a: int32): int32 { a + 1 }
}

func documentHighlight_test0094(): int64 {
    let x: int32  = plus94(1,1)
    let y: int32  = self94(1)
    if ( x == y ) {
        return 0
    }
    return 1
}
