// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func test_hover_test028(a:int32, b:int32): int32 {
    /**
    *test hover
    *test_hover_test028_in
    */
    func test_hover_test028_in():int32 {
        return 1
    }
    var value :int32  = test_hover_test028_in()
    return value + a + b
}

func run_hover_test028(): unit {
    var value :int32  = test_hover_test028(1,2)
}
