// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParamsMacro

import unusedDefaultParamsMacro.macroPkg.*

@PassThrough
public func macroScopeUnused(id: String, name!: String = "aaa") {
    println(id)
}

@PassThrough
public func macroScopeUsed(id: String, name!: String = "aaa") {
    println(id + name)
}

public func invokeMacroScope() {
    macroScopeUnused("id", name: "external")
    macroScopeUsed("id", name: "external")
}
