// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

trait Num931 {
    func plus93(a: This, b: This): This
}

trait Num931 {
    func plus93(a: This, b: This): This
}

trait Num932 {
    func self93(a: This): This
}

append int32 with Num931 ,Num932 {
    func plus93(a: int32, b: int32): int32 { a + b }
    func self93(a: int32): int32 { a + 1 }
}

func documentHighlight_test0093(): int64 {
    let x: int32  = plus93(1,1)
    let y: int32  = self93(1)
    if ( x == y ) {
        return 0
    }
    return 1
}
