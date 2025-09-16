// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package pkg
from std import ast.*

public macro AA(input: Tokens): Tokens {
    input
}

public macro BB(attr: Tokens, input: Tokens): Tokens {
    input
}

