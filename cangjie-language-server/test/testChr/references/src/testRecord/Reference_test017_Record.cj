// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testRecord

struct Record1{
    let width1: Int32 = 10
    var length1: Int32 = 20

    init (width1: Int32, length1: Int32) {
        this.width1 = width1
        this.length1 = length1
    }
}

func test_struct() = {
    let a = Record1(1,1)
    a.width1 = 1
}
