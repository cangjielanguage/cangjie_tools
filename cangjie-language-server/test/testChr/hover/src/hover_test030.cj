// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class test_hover_test030 {
    var aaa:int32 = 1
    /**
    *test hover
    *test_hover_test030 init(a:int32)
    */
    init(a:int32) {
        aaa = a
    }
    /**
    *test hover
    *test_hover_test030 init(a:int32, b:int32)
    */
    init(a:int32, b:int32) {
        aaa = a + b
    }
}

func run_hover_test030():unit {
    var value:test_hover_test030 = test_hover_test030(1)
    var value1:test_hover_test030 = test_hover_test030(1,2)
}
