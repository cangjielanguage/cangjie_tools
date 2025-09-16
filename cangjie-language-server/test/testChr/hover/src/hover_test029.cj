// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
*test hover
*test_hover_test029
*/
func test_hover_test029(a:int32, b:int32): int32 {
    /**
    *test hover
    *test_hover_test029_in
    */
    func test_hover_test029_in():int32 {
        return 1
    }
    var value :int32  = test_hover_test029_in()
    return value + a + b
}
