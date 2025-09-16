// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.ast.* a
struct VV<T, U> {
    init(a: T, b: U) {}
    func dd(a: T, b: U) {}
    func dd(a: Int64, b: U) {}
}
VV
macro
