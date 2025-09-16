// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package doc.CCC

import std.ast.*
public macro M2(input: Tokens): Tokens {
    var decl = (parseDecl(input) as ClassDecl).getOrThrow()
    return quote(class CM {$(decl.body.decls) var bbcc = 1;public func hh1(){}
    })
}
public macro M3(input: Tokens): Tokens {
    quote(func fooa(x:Float64){$input})
}
