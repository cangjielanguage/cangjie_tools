// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.CrossPkgTest

import ohos_app_cangjie_entry.Pkg1.*

class TEST_CLASS_ABSTRCT <: P1_ABSTRACT_C0 {}

class TEST_CLASS_VISIABLE <: P1_VISIABLE_C1 {
    public func C1_PUBLIC_F1(): Unit {
    	super.C1_PUBLIC_F1()
    }
    protected func C1_PROTECTED_F1(): Unit {
    	super.C1_PROTECTED_F1()
    }
}

class TEST_CLASS_ARGS <: P1_ARGS_C2 {}

open class TEST_CLASS_RET <: P1_RET_C3 {}

class TEST_CLASS_OVERRIDE <: P1_OVERLOAD_C4 {}

class TEST_CLASS_OVERRIDABLE <: P1_OVERRIDABLE_C5 {}

class TEST_CLASS_EXTEND_INTERFACE <: P1_VISIABLE_I0 {}

class TEST_CLASS_EXTEND_ABSTRCT_INTERFACE <: P1_ABSTRACT_C0 & P1_VISIABLE_I0 {}

class TEST_CLASS_EXTEND_CLASS_MULTI_INTERFACE <: P1_VISIABLE_C1 & P1_VISIABLE_I0 & P1_BODY_I1 {}

class TEST_CLASS_REOPEN <: P1_REOPEN_C6 {}

class TEST_CLASS_CLOSE <: P1_CLOSE_C7 {}

class TEST_CLASS_EXTEND_TEMPLATE_INTERFACE<K> <: P1_TEMPLATE_I5<K> {}

class TEST_CLASS_EXTEND_MULTI_TEMPLATE<K, V> <: P1_MULTI_TEMPLATE_I6<K, V> {}

class TEST_CLASS_EXTEND_TWICE <: TEST_CLASS_RET {}

open class TEST_CLASS_IMPLEMENTED_SOME <: P1_RET_C3 {
    public func C3_WITHRET_F3(): Int32 {
    	super.C3_WITHRET_F3()
    }
}

class TEST_CLASS_BLOCK <: TEST_CLASS_IMPLEMENTED_SOME {}
