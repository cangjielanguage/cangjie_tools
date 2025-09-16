// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package default.define
import std.ast.*

public macro TestD12(input: Tokens): Tokens {
    println("I'm in macro body")
    return input
}



