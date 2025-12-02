// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

@C
public struct X {
    let _cjbind_opaque_blob: UInt64

    public init() {
        this._cjbind_opaque_blob = unsafe { zeroValue<UInt64>() }
    }
}

@C
public struct _cjbind_ty_1 {
    public let length: Int32
    public let cproto: Int32

    public init(length: Int32, cproto: Int32) {
        this.length = length
        this.cproto = cproto
    }
}

@C
public struct _cjbind_ty_2 {
    public let name: CString
    public let base: Int32

    public init(name: CString, base: Int32) {
        this.name = name
        this.base = base
    }
}

@C
public struct _cjbind_ty_3 {
    public let tab: CPointer<JSCFunctionListEntry>
    public let len: Int32

    public init(tab: CPointer<JSCFunctionListEntry>, len: Int32) {
        this.tab = tab
        this.len = len
    }
}

@C
public struct _cjbind_ty_4 {
    let _cjbind_opaque_blob: VArray<UInt64, $2>

    public init() {
        this._cjbind_opaque_blob = unsafe { zeroValue<VArray<UInt64, $2>>() }
    }
}

@C
public struct JSCFunctionListEntry {}
