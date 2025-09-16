// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjie0625.test

class A_11123 {
    public prop testccc: Int64 {
        get() {
            123
        }
    }
}

func test_011() {
    var a_11 = A_11123()
    println(a_11.testccc) // 123
}

