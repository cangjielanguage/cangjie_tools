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
    public let __cjbind_anon_2: _cjbind_ty_1
    public let z: Int32
    public let __cjbind_anon_3: _cjbind_ty_1

    public init(a: Int32, b: Int32, __cjbind_anon_1: _cjbind_ty_1, __cjbind_anon_2: _cjbind_ty_1, z: Int32,
        __cjbind_anon_3: _cjbind_ty_1) {
        this.a = a
        this.b = b
        this.__cjbind_anon_1 = __cjbind_anon_1
        this.__cjbind_anon_2 = __cjbind_anon_2
        this.z = z
        this.__cjbind_anon_3 = __cjbind_anon_3
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
