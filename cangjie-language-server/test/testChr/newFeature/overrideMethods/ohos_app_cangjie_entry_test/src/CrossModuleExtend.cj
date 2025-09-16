// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry_test

import ohos_app_cangjie_entry.Pkg1.*

class Base {}

extend Base <: P1_BODY_I1 {}

extend Base <: P1_VISIABLE_I0 & P1_RET_I3 {}
