// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry_test

import ohos_app_cangjie_entry.Pkg1.*

enum TEST_ENUM_PROP <: P1_FOR_ENUM_I7 {
    PLACEHOLDER
}

enum TEST_ENUM_FUNC <: P1_RET_I3 {
    PLACEHOLDER
}

enum TEST_ENUM_MULTI_EXTEND <: P1_RET_I3 & P1_FOR_ENUM_I7 {
    PLACEHOLDER
}

interface ENUM_EXTEND_ONCE <: P1_RET_I3 {
    func ENUM_WITHBODY_F0(): Unit {}
}

enum TEST_ENUM_EXTEND_TWICE <: ENUM_EXTEND_ONCE {
    PLACEHOLDER
}
