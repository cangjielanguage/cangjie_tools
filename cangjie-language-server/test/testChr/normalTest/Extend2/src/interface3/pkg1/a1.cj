// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface3.pkg1

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

// The extension will not be exported because I1 is not visible outside the file.
extend<T> Foo<T> <: I1 {
    public func foo1() {}
}
// The extension is exported. Only the package that imports both Foo and I2 can access the members of I2 in the extension.
extend<T> Foo<T> <: I2 {
    public func foo2() {}
}
// The extension is exported, and only packages that import both Foo and I3 can access the members of I3 in the extension.
extend<T> Foo<T> <: I3 {
    public func foo3() {}
}

