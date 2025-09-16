// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.CrossPkgTest

import ohos_app_cangjie_entry.Pkg1.*

private interface TEST_INTERFACE_VISIABLE <: P1_VISIABLE_I0 {}

private interface TEST_INTERFACE_BODY <: P1_BODY_I1 {}

private interface TEST_INTERFACE_ARGS <: P1_ARGS_I2 {}

private interface TEST_INTERFACE_RET <: P1_RET_I3 {}

private interface TEST_INTERFACE_OVERLOAD <: P1_OVERLOAD_I4 {}

private interface TEST_INTERFACE_EXTENDED_ONCE <: P1_VISIABLE_I0 {
    func TEST_INTERFACE_EXTENDED_ONCE_F0(): Unit
    prop TEST_INTERFACE_EXTENDED_ONCE_P0: Int32 {
        get() { 0 }
    }
}

private interface TEST_INTERFACE_EXTENDED_ONCE_WITH_OVERRIDE <: P1_VISIABLE_I0 {
    func DEFAULT_FO(): Unit {

    }

    prop DEFAULT_PO: Int32 {
        get() { 0 }
    }
}

private interface TEST_INTERFACE_EXTENDED_TWICE <: TEST_INTERFACE_EXTENDED_ONCE {}

private interface TEST_INTERFACE_EXTENDED_TWICE_WITH_OVERRIDE <: TEST_INTERFACE_EXTENDED_ONCE_WITH_OVERRIDE {}

private interface TEST_INTERFACE_MULTI_EXTEND <: P1_VISIABLE_I0 & P1_BODY_I1 {}

