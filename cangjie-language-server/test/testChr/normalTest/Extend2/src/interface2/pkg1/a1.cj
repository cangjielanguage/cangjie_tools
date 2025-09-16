// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface2.pkg1

import cangjieextend.interface2.base.Foo

private interface I1 {
    func foo1(): Unit
}
internal interface I2 {
    func foo2(): Unit
}
protected interface I3 {
    func foo3(): Unit
}
public interface I4 {
    func foo4(): Unit
}

private interface F1 {}
internal interface F2 {}
protected interface F3 {}
public interface F4 {}

extend<T> Foo<T> <: I1 & I2 & I3 & I4 where T <: F2 {
    public func foo1() {}
    public func foo2() {}
    public func foo3() {}
    public func foo4() {}
}

private class C1 <: F2 {}

private let a113 = Foo<C1>().foo1() // ok
