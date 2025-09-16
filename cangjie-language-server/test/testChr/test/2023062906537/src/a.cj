// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface OP<T> {
    func op(op: T): Unit {}
    operator func <(rhs: T): Bool {
        true
    }
}

struct myString <: OP<myString> {
    public func op(op: myString) {}
    public operator func <(rhs: myString) {
        true
    }
}

func testmain() {
    let a = myString()
    let b = a.
}
