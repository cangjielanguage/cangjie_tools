// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

var ss = 1

class CC {
    static func test1() {}
    static prop aa: Int64 {
        get() {
            0
        }
    }
}

func fun() {
    var f1 = CC.test1()
    var a1 = CC.aa
}

