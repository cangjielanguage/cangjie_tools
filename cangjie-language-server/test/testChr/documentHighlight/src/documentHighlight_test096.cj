// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

trait Num961 {
    func plus96(a: This, b: This): This

}
trait Num962 
    func self96(a: This): This
}

append int32 with Num961 ,Num962 {
    func plus96(a: int32, b: int32): int32 { a + b }
    func self96(a: int32): int32 { a + 1 }
}

func documentHighlight_test0096(): int64 {
    let x: int32  = plus96(1,1)
    let y: int32  = self96(1)
    if ( x == y ) {
        return 0
    }
    return 1
}
