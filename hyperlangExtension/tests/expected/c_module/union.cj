package my_module

func __cjbind_read_Int32_from_UInt64(storage: UInt64): Int32 {
    var s = storage
    return unsafe { CPointer<Int32>(CPointer<UInt8>(inout s)).read() }
}

func __cjbind_write_Int32_from_UInt64(storage: UInt64, val: Int32): UInt64 {
    var s = storage
    unsafe { CPointer<Int32>(CPointer<UInt8>(inout s)).write(val) }
    return s
}

func __cjbind_read_CPointer_Unit__from_UInt64(storage: UInt64): CPointer<Unit> {
    var s = storage
    return unsafe { CPointer<CPointer<Unit>>(CPointer<UInt8>(inout s)).read() }
}

func __cjbind_write_CPointer_Unit__from_UInt64(storage: UInt64, val: CPointer<Unit>): UInt64 {
    var s = storage
    unsafe { CPointer<CPointer<Unit>>(CPointer<UInt8>(inout s)).write(val) }
    return s
}

func __cjbind_read_CString_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>): CString {
    var s = storage
    return unsafe { CPointer<CString>(CPointer<UInt8>(inout s)).read() }
}

func __cjbind_write_CString_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>, val: CString): VArray<UInt64, $2> {
    var s = storage
    unsafe { CPointer<CString>(CPointer<UInt8>(inout s)).write(val) }
    return s
}

func __cjbind_read_Int32_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>): Int32 {
    var s = storage
    return unsafe { CPointer<Int32>(CPointer<UInt8>(inout s)).read() }
}

func __cjbind_write_Int32_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>, val: Int32): VArray<UInt64, $2> {
    var s = storage
    unsafe { CPointer<Int32>(CPointer<UInt8>(inout s)).write(val) }
    return s
}

func __cjbind_read_Float64_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>): Float64 {
    var s = storage
    return unsafe { CPointer<Float64>(CPointer<UInt8>(inout s)).read() }
}

func __cjbind_write_Float64_from_VArray_UInt64__2_(storage: VArray<UInt64, $2>, val: Float64): VArray<UInt64, $2> {
    var s = storage
    unsafe { CPointer<Float64>(CPointer<UInt8>(inout s)).write(val) }
    return s
}

// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
@C
public struct X {
    var _storage: UInt64 = unsafe { zeroValue<UInt64>() }
    public init() {}
    public mut prop a: Int32 {
        get() {
            return __cjbind_read_Int32_from_UInt64(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_Int32_from_UInt64(this._storage, value)
        }
    }
    public mut prop ptr: CPointer<Unit> {
        get() {
            return __cjbind_read_CPointer_Unit__from_UInt64(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_CPointer_Unit__from_UInt64(this._storage, value)
        }
    }
}

@C
public struct _cjbind_ty_1__cjbind_ty_1 {
    // XXX: should move outside union
    public var length: Int32
    // XXX: should move outside union
    public var cproto: Int32
    public init(length: Int32, cproto: Int32) {
        this.length = length
        this.cproto = cproto
    }
}

@C
public struct _cjbind_ty_1__cjbind_ty_2 {
    public var name: CString
    public var base: Int32
    public init(name: CString, base: Int32) {
        this.name = name
        this.base = base
    }
}

@C
public struct _cjbind_ty_1__cjbind_ty_3 {
    public var tab: CPointer<JSCFunctionListEntry>
    public var len: Int32
    public init(tab: CPointer<JSCFunctionListEntry>, len: Int32) {
        this.tab = tab
        this.len = len
    }
}

@C
public struct _cjbind_ty_1 {
    var _storage: VArray<UInt64, $2> = unsafe { zeroValue<VArray<UInt64, $2>>() }
    public init() {}
    public mut prop str: CString {
        get() {
            return __cjbind_read_CString_from_VArray_UInt64__2_(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_CString_from_VArray_UInt64__2_(this._storage, value)
        }
    }
    public mut prop i32: Int32 {
        get() {
            return __cjbind_read_Int32_from_VArray_UInt64__2_(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_Int32_from_VArray_UInt64__2_(this._storage, value)
        }
    }
    public mut prop i64: Int32 {
        get() {
            return __cjbind_read_Int32_from_VArray_UInt64__2_(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_Int32_from_VArray_UInt64__2_(this._storage, value)
        }
    }
    public mut prop f64: Float64 {
        get() {
            return __cjbind_read_Float64_from_VArray_UInt64__2_(this._storage)
        }
        set(value) {
            this._storage = __cjbind_write_Float64_from_VArray_UInt64__2_(this._storage, value)
        }
    }
}

@C
public struct JSCFunctionListEntry {
    let _cjbind_empty: UInt8
    public init(_cjbind_empty: UInt8) {
        this._cjbind_empty = _cjbind_empty
    }
}
