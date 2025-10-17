// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import cjpm.config.*

func supportLto(): Bool {
    if (crossCompileTarget.isEmpty()) {
        return targetConfigName.contains("linux") || targetConfigName.contains("ohos") ||
        targetConfigName.contains("android")
    }
    return crossCompileTarget.contains("linux") || crossCompileTarget.contains("ohos") ||
        crossCompileTarget.contains("android")
}