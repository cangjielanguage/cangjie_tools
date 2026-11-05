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

// 定义包含柔性数组成员的结构体
@C
public struct FlexibleString {
    // 字符串长度
    public var length: Int32
    // 柔性数组成员，存储字符串数据
    public let data: VArray<UInt8, $0> = unsafe { zeroValue<VArray<UInt8, $0>>() }
    public init(length: Int32) {
        this.length = length
    }
}

public unsafe func FlexibleString_data_ptr(value: CPointer<FlexibleString>): CPointer<UInt8> {
    return CPointer<UInt8>(CPointer<UInt8>(value) + 4)
}

public unsafe func FlexibleString_data_view(value: CPointer<FlexibleString>, count!: Int64): __CjbindArrayView<UInt8> {
    return __CjbindArrayView<UInt8>(pointer: FlexibleString_data_ptr(value), size: count)
}
