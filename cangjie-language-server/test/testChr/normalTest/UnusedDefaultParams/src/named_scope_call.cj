// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParams

public func invokeCrossScope() {
    crossScopeUnused("id", name: "external")
    crossScopeUnused("id")
    crossScopeRequired("id", name: "external")
    crossScopeUsed("id", name: "external")
    CrossScopeMethod().run("id", name: "external")
    CrossScopeConstructor("id", name: "external")
}
