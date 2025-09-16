// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface Sizeable {
    func size1(): Unit
}

extend<T> Array<T> <: Sizeable {
    public func size1(): Unit {}
}

let c1: Sizeable = Array<Int64>()
let c2 = c1.size1()
