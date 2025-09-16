// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.C

import default.B.*

// 二级孙子类继承场景  Scenarios4
public open class CClass <: BClass {
    public open func funcAold(): Unit {
        return
    }
    func f1(a: Int64): Unit {
        return
    }
    public prop propA: Int64 {
        get() {
            1
        }
    }
}
var b = CClass().funcAold()

