// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
*test hover
*testpkg sum_hover
*/
let sum_hover26 = (a: int32, b: int32): int32 => {a + b}

func test_hover_test026(): unit {
    var value :int32  = sum_hover26(1,2)
}
