// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/*
函数入口行：6
函数出口行：8 12
*/
func test4() {
    let x = -10
    match (x) {
        case 0 => "zero"
        case n =>
            let n = 0
            println(n)
            "x is not zero"
    }
}
