// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.D

import default.C.*

// 三级子类继承场景  Scenarios5
public open class DClass <: CClass {
    public func funcAold(): Unit {
        return
    }
}

var b = DClass().funcAold()

