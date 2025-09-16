// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

struct RR1 {
    let width11: Int32 = 10
    let length1: Int32 = 20
    var test_var: Int32 = 30
    let perimeter1: () -> Int32 = { => 2 * (width11 + length1) }
    init (width1: Int32, length1: Int32) {
        this.width11 = width1
        this.length1 = length1
    }
    init (side: Int32) {
        this.width11 = side
        this.length1 = side
    }
    func area3(): Int32 = width11 * length1

    func area2(): Int32 = {
        this.test_var = 10 // Record成员函数函数体内只能访问let定义的成员变量检查 ??
        test_var = 10
        var a = this.width11
        // 要用this吗？
        return 1
    }
}

func test_r1() = {
    let r1: RR1 = RR1(10)

    r1.area3()
    r1.width11

}
