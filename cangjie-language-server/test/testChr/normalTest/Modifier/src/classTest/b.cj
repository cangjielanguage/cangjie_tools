// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.classTest

import default.classPkg.*

import default.classP1.*

import default.classP2.*

func f() {
    b = 0
    let a = A() // classP1.A can not access
    let c = B()
    return 0
}

class D2 <: D1 {
    func f() {
        d1 = 2
    }
}
