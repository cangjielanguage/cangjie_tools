// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I1 {
	static func f1(): Unit
}

interface I2 {
    public func f2(): Unit
}

interface I3 {
    private func f3(): Unit
}

let xI = I1.f1() // Error, cannot directly call
