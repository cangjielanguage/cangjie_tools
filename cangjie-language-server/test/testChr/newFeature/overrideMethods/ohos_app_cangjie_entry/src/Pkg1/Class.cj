// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.Pkg1

public abstract class P1_ABSTRACT_C0 {
    public open func C0_NOBODY_F0(): Unit
    public open func C0_WITHBODY_F0(): Unit {}

    public open prop C0_NOBODY_P0: Int32
    public open prop C0_WITHBODY_P0: Int32 {
        get() { 0 }
    }
    public open mut prop C0_NOBODY_MUT_P0: Int32
    public open mut prop C0_WITHBODY_MUT_P0: Int32 {
        get() { 0 }
        set(v) { }
    }
}

public open class P1_VISIABLE_C1 {
    protected open func C1_PROTECTED_F1(): Unit {}
    public open func C1_PUBLIC_F1(): Unit {}
}

public open class P1_ARGS_C2 {
    public open func C2_NOARGS_F2(): Unit {}
    public open func C2_WITHARG_UNNAMED_F2(arg0: Int32): Unit {}
    public open func C2_WITHARG_NAMED_F2(arg0!: Int32): Unit {}
    public open func C2_WITHARG_MULTI_F2(arg0: Int32, arg1: String): Unit {}
}

public open class P1_RET_C3 {
    public open func C3_NORET_F3(): Unit {}
    public open func C3_WITHRET_F3(): Int32 { 0 }
}

public open class P1_OVERLOAD_C4 {
    public open func C4_OVERLOAD_F4(): Unit {}
    public open func C4_OVERLOAD_F4(arg0: Int32): Unit {}
    public open func C4_OVERLOAD_F4(arg0: Int32, arg1: String): Unit {}
}

public open class P1_OVERRIDABLE_C5 {
    public open func C5_OPEN_F5(): Unit {}
    public func C5_NOOPEN_F5(): Unit {}

    public open prop C5_OPNE_P5: Int32 {
        get() { 0 }
    }
    public prop C5_NOOPEN_P5: Int32 {
        get() { 0 }
    }
}

public open class P1_REOPEN_C6 <: P1_VISIABLE_C1 {
    public open func C1_PUBLIC_F1(): Unit {}
}

public open class P1_CLOSE_C7 <: P1_VISIABLE_C1 {
    public func C1_PUBLIC_F1(): Unit {}
}
