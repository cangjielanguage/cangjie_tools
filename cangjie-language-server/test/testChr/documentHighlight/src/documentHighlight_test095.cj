// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

trait Num951 {
    func plus95(a: This, b: This): This


trait Num952 {
    func self95(a: This): This
}

append int32 with Num951 ,Num952 {
    func plus95(a: int32, b: int32): int32 { a + b }
    func self95(a: int32): int32 { a + 1 }
}

func documentHighlight_test0095(): int64 {
    let x: int32  = plus95(1,1)
    let y: int32  = self95(1)
    if ( x == y ) {
        return 0
    }
    return 1
}
