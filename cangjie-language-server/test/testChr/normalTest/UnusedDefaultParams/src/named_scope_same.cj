// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParams

public enum NamedScopeMode <: ToString {
    | Active
    | Idle
    public func toString(): String {
        let name = "active"
        match (this) {
            case Active => name
            case Idle => "idle"
        }
    }
}

public func namedScopeEnum(id: String, name!: String = NamedScopeMode.Active.toString()) {
    println(id)
}

public func namedScopeRequired(id: String, name!: String) {
    println(id)
}

public func namedScopeCapture(id: String, name!: String = "aaa") {
    let read = { => id + name }
    println(read())
}

public func namedScopeForward(id: String, name!: String = "aaa") {
    namedScopeEnum(id, name: name)
}

public func namedScopeRecursive(id: String, name!: String = "aaa"): Unit {
    if (id.isEmpty()) { return }
    namedScopeRecursive("", name: "external")
}

public func invokeNamedScope() {
    namedScopeEnum("id", name: "external")
    namedScopeEnum("id")
    namedScopeRequired("id", name: "external")
    namedScopeCapture("id", name: "external")
    namedScopeForward("id", name: "external")
    namedScopeRecursive("id", name: "external")
}
