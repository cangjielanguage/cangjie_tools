// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package MacroNodeCompletion.pkg

import std.ast.*

public macro State(input: Tokens): Tokens {
    return quote(
        private prop a: String {
            get() {""}
        }
    )
}
