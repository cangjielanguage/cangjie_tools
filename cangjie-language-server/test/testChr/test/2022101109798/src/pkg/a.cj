// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package pkg
from std import ast.*

public macro testMacro111(input: Tokens): Tokens {
    input
}

public macro testMacro222(attr: Tokens, input: Tokens): Tokens {
    input
}
