// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.Pkg1

struct TEST_STRUCT <: P1_BODY_I1 {}

struct TEST_STRUCT_MUTIL_EXTEND <: P1_RET_I3 & P1_BODY_I1 {}

interface STRUCT_EXTEND_ONCE <: P1_RET_I3 {
    func ENUM_WITHBODY_F0(): Unit {}
}

struct TEST_STRUCT_EXTEND_TWICE <: STRUCT_EXTEND_ONCE {}
