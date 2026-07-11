// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import script_dep.scriptOnlyFunc

func useScriptDepInBuild(): Int64 {
    let localValue = scriptOnlyFunc()
    return local
}
