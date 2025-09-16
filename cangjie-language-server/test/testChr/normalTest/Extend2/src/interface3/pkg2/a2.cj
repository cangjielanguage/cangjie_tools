// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface3.pkg2

import cangjieextend.interface3.base.Foo

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

// The extension is exported. Only the package that imports both Foo and (I2/I3) can access the members of the corresponding (I2/I3) in the extension.
extend<T> Foo<T> <: I1 & I2 & I3 {
    public func foo1() {}
    public func foo2() {}
    public func foo3() {}
}

class D{}
let b1 = Foo<D>().foo1()
