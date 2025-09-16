// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

struct SS {
    var a: Int64 = 1
    init(a: Int64) {
        this.a
    }
}

interface JJ {
    func test(j: SS) {
        j.a
    }
}

enum EE {
    | A | B
    func test(e: SS) {
        e.a
    }
}

extend EE {
    func test1(t: SS) {
        t.a
    }
}

let b = { l: SS => l.a }

func test() {
    var a: SS = SS(1)
    while (true) {
        var b = (1, a.a)
    }
}

