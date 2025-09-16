// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.collection.*
let c1 = HashSet<Int64>([0, 1, 2])

type va1 = HashSet<Int64>
let c2: va1 = va1([2, 3])


class C {
    static let a = 1
    C(a:Int64){}
    init(){}
}

type ccc = C;
let c6:C = C(1);
let c7 : ccc = ccc(1);
let cccc = ccc.a
