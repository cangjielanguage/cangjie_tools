// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.direct2.pkg1

private interface I1 {}
internal interface I2 {}
public interface I3 {}

extend Int32 <: I2 {}
extend Int64 <: I3 {}

private class A1<T>{}
internal class A2<T>{}
public class A3<T> {}

extend<T> A3<T> where T <: I1 {
    public func foo1() {}
    private func foo3(a: T): Unit {}
} // not exported.

extend<T> A3<T> where T <: I2 {
    public func foo2() {}
} // child exported.

extend<T> A3<T> where T <: I3 { // exported.
    public func foo3(a: T, b: Int32): Unit {}
}

extend<T> A1<T> where T <: I3 {// not exported.
    public func foo23() {}
}

extend<T> A2<T> where T <: I3 {// child exported.
    public func foo33() {}
    private func fooPrivate33() {}
}

let a3 = A3<Int32>().foo2() // ok
let a4 = A3<Int64>().foo3(1, 2) // ok

