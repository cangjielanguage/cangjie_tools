// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package MacroNodeCompletion.pkg

import std.ast.*

public macro Component(input: Tokens): Tokens {
    let classDecl = (parseDecl(input) as ClassDecl).getOrThrow()
    let classBody = classDecl.body
    var members = Tokens()
    for (item in classBody.decls) {
        members = members + quote($item)
    }
    quote(
        public class A1 {}
        public class A2 {
            $members
        }
    )
}

