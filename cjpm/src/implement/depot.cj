// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

// used for central repository
public class Depot {
    // used for central repository
    public var base_url: String = ""

    // used for central repository
    public func loadConfiguration(): Bool {
        return false
    }

    // used for central repository
    public static func new(): Option<Depot> {
        let instance: Depot = Depot()
        if (instance.loadConfiguration()) {
            return Option<Depot>.Some(instance)
        }
        return Option<Depot>.None
    }

    // used for central repository
    public func getLocalRepo(): String {
        return ""
    }

    // used for central repository
    public func download(_: String, _: String): (Bool, String) {
        return (false, "")
    }
}
