// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Data {
    var cc = 0
    let d: String = "hello"
    Data(let abcd!: Int64 = 1) {}
    func bar(): String {
        "bar"
    }
}

var a1 = Data(abcd: 1).bar()
