package my_module

// A bounded view over a C flexible or zero-length array. Construction from an owning pointer and indexing stay unsafe because pointer validity and lifetime are caller contracts.
public struct __CjbindArrayView<T> where T <: CType {
    let _pointer: CPointer<T>
    let _size: Int64

    public init(pointer!: CPointer<T>, size!: Int64) {
        this._pointer = pointer
        this._size = size
        if (size < 0) {
            throw Exception("array view size cannot be negative")
        }
    }

    public prop pointer: CPointer<T> {
        get() {
            return this._pointer
        }
    }

    public prop size: Int64 {
        get() {
            return this._size
        }
    }

    public prop isEmpty: Bool {
        get() {
            return this._size == 0
        }
    }

    public unsafe operator func [](index: Int64): T {
        if (index < 0 || index >= this._size) {
            throw Exception("array view index out of bounds")
        }
        return this._pointer.read(index)
    }

    public unsafe operator func [](index: Int64, value!: T): Unit {
        if (index < 0 || index >= this._size) {
            throw Exception("array view index out of bounds")
        }
        this._pointer.write(index, value)
    }
}

public struct __CjbindConstArrayView<T> where T <: CType {
    let _pointer: CPointer<T>
    let _size: Int64

    init(pointer!: CPointer<T>, size!: Int64) {
        this._pointer = pointer
        this._size = size
        if (size < 0) {
            throw Exception("array view size cannot be negative")
        }
    }

    public prop size: Int64 {
        get() {
            return this._size
        }
    }

    public prop isEmpty: Bool {
        get() {
            return this._size == 0
        }
    }

    public unsafe operator func [](index: Int64): T {
        if (index < 0 || index >= this._size) {
            throw Exception("array view index out of bounds")
        }
        return this._pointer.read(index)
    }
}

@C
public struct Point__cjbind_ty_1 {
    public var x: Int32
    public var y: Int32
    public init(x: Int32, y: Int32) {
        this.x = x
        this.y = y
    }
}

// 外层结构体包含匿名结构体
@C
public struct Point {
    // 匿名结构体：无名称，直接包含 x、y 成员
    public var a: Int32
    public var b: Int32
    public let __cjbind_anon_1: Point__cjbind_ty_1
    // 外层结构体自身的成员
    public var z: Int32
    public init(a: Int32, b: Int32, __cjbind_anon_1: Point__cjbind_ty_1, z: Int32) {
        this.a = a
        this.b = b
        this.__cjbind_anon_1 = __cjbind_anon_1
        this.z = z
    }
}

@C
public struct person {
    public var age: Int32
    public init(age: Int32) {
        this.age = age
    }
}

@C
public struct Point3D {
    public var x: Int64
    public var y: Int64
    public var z: Int64
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
    public var total: Int32
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
    public let b: VArray<Int32, $0> = unsafe { zeroValue<VArray<Int32, $0>>() }
    public init(a: VArray<Int32, $2>) {
        this.a = a
    }
}

public unsafe func S_b_ptr(value: CPointer<S>): CPointer<Int32> {
    return CPointer<Int32>(CPointer<UInt8>(value) + 8)
}

public unsafe func S_b_view(value: CPointer<S>, count!: Int64): __CjbindArrayView<Int32> {
    return __CjbindArrayView<Int32>(pointer: S_b_ptr(value), size: count)
}

@C
public struct Data {
    public var a: Int32
    public var b: Int32
    public init(a: Int32, b: Int32) {
        this.a = a
        this.b = b
    }
}

foreign func getData(a: Int32, b: Int32): Data

@C
public struct teststruct {
    public var i8: Int32
    public var ui8: UInt32
    public init(i8: Int32, ui8: UInt32) {
        this.i8 = i8
        this.ui8 = ui8
    }
}

@C
public struct Point1__cjbind_ty_1 {
    public var x: Int32
    public var y: Int32
    public init(x: Int32, y: Int32) {
        this.x = x
        this.y = y
    }
}

@C
public struct Point1__cjbind_ty_2 {
    public var m: Int32
    public var n: Int32
    public init(m: Int32, n: Int32) {
        this.m = m
        this.n = n
    }
}

@C
public struct Point1 {
    public let test1: Point1__cjbind_ty_1
    public let test2: Point1__cjbind_ty_2
    public init(test1: Point1__cjbind_ty_1, test2: Point1__cjbind_ty_2) {
        this.test1 = test1
        this.test2 = test2
    }
}
