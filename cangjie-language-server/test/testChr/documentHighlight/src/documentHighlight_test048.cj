// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

trait Num1 {
    func plus(a: This, b: This): This
}

trait Num2 {
    func self(a: This): This
}

append int32 with Num1 ,Num2 {
    func plus(a: int32, b: int32): int32 { a + b }
    func self(a: int32): int32 { a + 1 }
}

func run1(): int64 {
    let x: int32  = plus(1,1)
    let y: int32  = self(1)
    if ( x == y ) {
        return 0
    }
    return 1
}
