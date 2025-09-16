// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.sync.*

class A {
    init() {}
}
func test1() {
    var obj1 = AtomicBool(true)
    var aInt8 : AtomicInt8 = AtomicInt8(10)
    var aInt16 : AtomicInt16 = AtomicInt16(10)
    var aInt32 : AtomicInt32 = AtomicInt32(10)
    var aInt64 : AtomicInt64 = AtomicInt64(10)
    var aUInt8 : AtomicUInt8 = AtomicUInt8(10)
    var aUInt16 : AtomicUInt16 = AtomicUInt16(10)
    var aUInt32: AtomicUInt32 = AtomicUInt32(10)
    var aUInt64 : AtomicUInt64 = AtomicUInt64(10)

    var error1 = AtomicInt64(10)
    var test1 = Atom
    var x = obj1.load() // b：true，类型为 Bool

    var t1 = A()
    var obj2r = AtomicReference(t1)
    var x1 = obj2r.load() // x 和 t1 是同一个对象
    var t2 = A()
    var y = obj2r.compareAndSwap(t2, A()) // t2 和 t1 不是同一个对象，CAS 失败，y:false
    y = obj2r.compareAndSwap(t1, t2) // CAS 成功，y：true
}
