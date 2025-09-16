// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.collection.*

main(): Int64 {
    let x: (VArray<Int64, $3>, ABC)
    let y: (VArray<ABC, $3>, Int64)
    let z: (VArray<VArray<VArray<ABC, $5>, $4>, $3>, Int64)
    let q = VArray<Int64, $5>(repeat: 0)
    let p = VArray<ABC, $5>(repeat: ABC())
    let arr1: VArray<ABC, $3> = [ABC(), ABC(), ABC()]
    let arr2: VArray<Int64, $3> = [1,2,3]
    let arr3: VArray<Array<Int64>, $5>
    let num = arr2.size
    let b: Map<Int64, Int64>
    return 0
}
class ABC {}
