// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.Pkg1

public interface P1_VISIABLE_I0 {
    func I0_DEFAULT_F0(): Unit
    prop I0_DEFAULT_P0: Int32
}

public interface P1_BODY_I1 {
    func I1_NOBODY_F1(): Unit
    func I1_WITHBODY_F1(): Unit {}

    prop I1_NOBODY_P1: Int32
    mut prop I1_NOBODY_MUT_P1: Int32
    prop I1_WITHBODY_P1: Int32 {
        get() { 0 }
    }
    mut prop I1_WITHNOBODY_MUT_P1: Int32 {
        get() { 0 }
        set(v) {}
    }
}

public interface P1_ARGS_I2 {
    func I2_NOARG_F2(): Unit
    func I2_WITHARG_UNNAMED_F2(arg0: Int32): Unit
    func I2_WIHTARG_NAMED_F2(arg0!: Int32): Unit
    func I2_WITHARG_MULIT_F2(arg0: Int32, arg1!: Int32): Unit
}

public interface P1_RET_I3 {
    func I3_NORET_F3(): Unit
    func I3_WITHRET_F3(): Int32
}

public interface P1_OVERLOAD_I4 {
    func I4_OVERRIDE_F4(): Unit
    func I4_OVERRIDE_F4(arg0: Int32): Unit
    func I4_OVERRIDE_F4(arg0: Int32, arg1: Int32): Unit
}

public interface P1_TEMPLATE_I5<T> {
    func I5_TEMPLATE_NO_ARG_F5(): Unit
    func I5_TEMPLATE_RET_F5(): T
    func I5_TEMPLATE_UNNAME_ARG_F5(arg0: T): Unit
}

public interface P1_MULTI_TEMPLATE_I6<K, V> {
    func I5_MULTI_TEMPLATE_F6(arg0: Iterable<K>, arg1: Iterable<V>): Unit
}

public interface P1_FOR_ENUM_I7 {
    prop I7_PROP: Int32
}
