// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface4.pkg8
import cangjieextend.interface4.pkg7.Foo1

public interface I {
    func f(): Unit
}

extend Foo1 {
    public func f() {}
}

// Abstract functions inherited from interface extensions cannot be covered or implemented with members of other non-exported extensions.
extend Foo1 <: I { 
}
