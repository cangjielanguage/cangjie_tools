// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.util

import std.collection.*

public class CliFailError <: Exception {
    public CliFailError(error: String) {
        super(error)
    }

    public init(errors: Iterable<String>) {
        this(String.join(errors |> collectArray, delimiter: '\n\n'))
    }
}

@When[debug]
public func assertion(lambda: () -> Bool, errmesg: () -> String): Unit {
    var res: ?String = None
    var hasFailed = true
    try {
        hasFailed = !lambda()
    } catch (e: Exception) {
        res = "${e}\n${errmesg()}"
    }
    if (hasFailed) {
        let output = res ?? errmesg()
        throw Exception("It's caught!: ${output}")
    }
}

@When[!debug]
public func assertion(_: () -> Bool, _: () -> String): Unit {
}

// The following are additional checks util which should be used only inside assertion call
public func checkAllUnique<T>(it: Iterable<T>): Bool where T <: Hashable & Equatable<T> {
    let n = it |> count
    let m = HashSet(it |> collectArray) |> count
    return n == m
}
