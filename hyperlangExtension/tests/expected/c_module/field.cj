// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

@C
public struct X {
    let _cjbind_opaque_blob: UInt32

    public init() {
        this._cjbind_opaque_blob = unsafe { zeroValue<UInt32>() }
    }
}
