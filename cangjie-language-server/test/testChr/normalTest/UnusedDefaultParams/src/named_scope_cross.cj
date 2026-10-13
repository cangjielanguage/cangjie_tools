// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParams

public func crossScopeUnused(id: String, name!: String = "aaa") {
    println(id)
}

public func crossScopeRequired(id: String, name!: String) {
    println(id)
}

public func crossScopeUsed(id: String, name!: String = "aaa") {
    println(id + name)
}

public class CrossScopeMethod {
    public func run(id: String, name!: String = "aaa") {
        println(id)
    }
}

public class CrossScopeConstructor {
    public init(id: String, name!: String = "aaa") {
        println(id)
    }
}
