// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class C {
    let c = 1
}

func foo3() {
    let bob = C()
    let s = match (bob) {
        case C => {
            => bob.c
        }
    }
}
