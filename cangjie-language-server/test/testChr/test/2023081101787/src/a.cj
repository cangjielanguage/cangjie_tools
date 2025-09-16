// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I1 {
    func h1(a: Rune) {}
}
class C12 <: I1 {
    func h1(a: Int64) {}
}

let m1: Int64 = 1
let m2 = C12()
let m3: Rune = 'a'

var m4 = m1 |> m2.h1
var m5 = m3 |> m2.h1

