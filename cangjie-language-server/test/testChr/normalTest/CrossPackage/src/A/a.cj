// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.A

public interface AInterface {
    func funcAold(): Unit
    prop propA: Int64 {
        get() {
            1
        }
    }
}

// 本包继承场景
open class AClass <: AInterface {
    public func funcAold(): Unit {
        return
    }
}
let aaa = 1

var b = AClass().funcAold()

