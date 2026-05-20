// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package primaryTyExtend.extends

public interface I1 {
    func foo1(): Unit
}

public interface I2 {
    func foo2(): Unit
}

public interface I3 {
    func foo3(): Unit
}

public interface I4 {
    func foo4(): Unit
}

extend Int <: I1 {
    public func foo1(): Unit {
    }
}

extend UInt <: I2 {
    public func foo2(): Unit {
    }
}

extend Int8 <: I3 {
    public func foo3(): Unit {
    }
}

extend Float16 <: I4 {
    public func foo4(): Unit {
    }
}

extend Rune <: I1 {
    public func foo1(): Unit {
    }
}

extend Bool <: I2 {
    public func foo2(): Unit {
    }
}