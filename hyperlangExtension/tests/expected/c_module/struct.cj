// cjbind 0.2.7 生成，请勿编辑, DO NOT EDIT

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
