// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package A

from std import ast.*

public macro M1(inputs: Tokens): Tokens {
    return inputs
}

public macro M3(input: Tokens): Tokens {
    return quote(
        class Useless {

        }
    )
}

