// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

public interface TomlType <: ToString {
    func typeString(): String
}

struct TomlBaseType <: TomlType & Equatable<TomlBaseType> {
    let btype: String

    public init(btype: String) {
        this.btype = btype
    }

    public func toString(): String {
        return typeString()
    }

    public func typeString(): String {
        return btype
    }

    public operator func ==(rhs: TomlBaseType): Bool {
        return btype == rhs.btype
    }

    public operator func !=(rhs: TomlBaseType): Bool {
        return btype != rhs.btype
    }
}
