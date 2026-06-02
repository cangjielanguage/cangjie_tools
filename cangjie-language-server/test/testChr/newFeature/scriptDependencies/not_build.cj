// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package script_dep_root

import script_dep.scriptOnlyFunc

public func useScriptDepInRoot(): Int64 {
    scriptOnlyFunc()
    return 0
}
