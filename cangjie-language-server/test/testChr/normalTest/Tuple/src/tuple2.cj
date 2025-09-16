// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


// 代码诊断
func test1() {
    let tuplePIE = (3.14, "PIE")     // The type of tuplePIE is inferred to be Float64*String.
    var a = tuplePIE[0]
    var b = tuplePIE[2]

    let tuple1 =(1, 2)
    tuple1 = (2, 3, 4)
}
