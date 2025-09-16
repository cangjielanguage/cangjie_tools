// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkgB.aaaa
import pkgA.*

public interface Cloneable<T> {
    func deepClone(): T
}



extend A <: Cloneable<Int64> {
    public func public_deepClone(): Int64 {
        return 1
    }
    public func deepClone(): Int64 {
        return 1
    }
    protected func protect_deepClone(): Int64 {
        return 1
    }
    private func private_deepClone(): Int64 {
        return 1
    }
    func default_deepClone(): Int64 {
        return 1
    }
}
let sasa = A().


