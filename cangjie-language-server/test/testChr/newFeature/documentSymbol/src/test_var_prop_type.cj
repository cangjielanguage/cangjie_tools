// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.time.*

var aa1 = 1
let aa2: Bool = false

class C111 {
    prop a: Int64 {
        get() {
            0
        }
    }
    mut prop b: Int64 {
        get() {
            0
        }
        set(v) {}
    }
}

var aa3 = C111()
let c = DateTime.now()
let ddd = c.toString()
var aaa = "${c.toString()}"
type ty1 = Int64
type ty2 = C111

