// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class A {
    public func foo() {

    }
}

func lam(a: Int64, b!: (A) -> Unit, c!: (A) -> Unit) {

}

func foo() {
    lam(1, b: {aa: A => aa. }) { bb: A => bb.foo() }
}
