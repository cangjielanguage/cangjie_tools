// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

@C
public struct _cjbind_ty_1 {
    public let x: Int32
    public let y: Int32

    public init(x: Int32, y: Int32) {
        this.x = x
        this.y = y
    }
}

@C
public struct Point {
    public let a: Int32
    public let b: Int32
    public let __cjbind_anon_1: _cjbind_ty_1
    public let z: Int32

    public init(a: Int32, b: Int32, __cjbind_anon_1: _cjbind_ty_1, z: Int32) {
        this.a = a
        this.b = b
        this.__cjbind_anon_1 = __cjbind_anon_1
        this.z = z
    }
}

@C
public struct person {
    public let age: Int32

    public init(age: Int32) {
        this.age = age
    }
}

@C
public struct Point3D {
    public let x: Int64
    public let y: Int64
    public let z: Int64

    public init(x: Int64, y: Int64, z: Int64) {
        this.x = x
        this.y = y
        this.z = z
    }
}

@C
public struct people {
    public let a: person
    public let b: person
    public let height: VArray<Int32, $2>
    public let total: Int32

    public init(a: person, b: person, height: VArray<Int32, $2>, total: Int32) {
        this.a = a
        this.b = b
        this.height = height
        this.total = total
    }
}

@C
public struct S {
    public let a: VArray<Int32, $2>
    public let b: VArray<Int32, $0>

    public init(a: VArray<Int32, $2>, b: VArray<Int32, $0>) {
        this.a = a
        this.b = b
    }
}

@C
public struct Data {
    public let a: Int32
    public let b: Int32

    public init(a: Int32, b: Int32) {
        this.a = a
        this.b = b
    }
}

foreign func getData(a: Int32, b: Int32): Data

@C
public struct teststruct {
    public let i8: Int32
    public let ui8: UInt32

    public init(i8: Int32, ui8: UInt32) {
        this.i8 = i8
        this.ui8 = ui8
    }
}

@C
public struct _cjbind_ty_2 {
    public let x: Int32
    public let y: Int32

    public init(x: Int32, y: Int32) {
        this.x = x
        this.y = y
    }
}

@C
public struct _cjbind_ty_3 {
    public let m: Int32
    public let n: Int32

    public init(m: Int32, n: Int32) {
        this.m = m
        this.n = n
    }
}

@C
public struct Point1 {
    public let __cjbind_anon_1: _cjbind_ty_2
    public let test1: _cjbind_ty_2
    public let __cjbind_anon_2: _cjbind_ty_3
    public let test2: _cjbind_ty_3

    public init(__cjbind_anon_1: _cjbind_ty_2, test1: _cjbind_ty_2, __cjbind_anon_2: _cjbind_ty_3, test2: _cjbind_ty_3) {
        this.__cjbind_anon_1 = __cjbind_anon_1
        this.test1 = test1
        this.__cjbind_anon_2 = __cjbind_anon_2
        this.test2 = test2
    }
}
