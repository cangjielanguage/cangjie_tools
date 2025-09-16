// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package tesrt4.AAA

import std.ast.*

public macro AA3(attr: Tokens, input: Tokens) {
    quote($attr;$input)
}

