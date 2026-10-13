// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParams

public class DefaultMethod {
    public func combine(id: String, name!: String = "aaa") {
        println(id + name)
    }
}

public func defaultGeneric<T>(id: T, name!: String = "aaa"): T {
    println(name)
    return id
}

public class DefaultConstructor {
    public init(id: String, name!: String = "aaa") {
        let joined = id + name
        println(joined)
    }
}

public class DefaultPrimaryMember {
    public DefaultPrimaryMember(public let id: String, public let name!: String = "aaa") {}
}
