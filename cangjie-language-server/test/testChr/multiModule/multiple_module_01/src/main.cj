// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import test.aoo.*
import test.boo.*
import pro1.xoo.*
import pro1.yoo.*
import pro2.zoo

main(): Int64 {
    let a1 = aoo.one
    let a2 = aoo.
    let a3 = aoo.o
    let b1 = boo.two
    let e1 = xoo.x
    let f1 = yoo.y
    let g1 = zoo.z
    let res = a1 + b1
    let res2 = e1 + f1 + g1
    if (res != 3 || res2 != 101) {
        return 1
    }
    return 0
}

