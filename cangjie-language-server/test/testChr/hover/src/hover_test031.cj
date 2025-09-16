// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class test_hover_test031 {
    var aaa:int32 = 1
    /**
    *test hover
    *test_hover_test031 init(a:int32)
    */
    init(a:int32) {
        aaa = a
    }
    /**
    *test hover
    *test_hover_test031 add(a:int32, b:int32)
    */
    func add(a:int32, b:int32):int32 {
        return a + b
    }

    /**
    *test hover
    *test_hover_test031 add(a:int32)
    */
    func add(a:int32):int32 {
        return a + aaa
    }
}

func run_hover_test031():unit {
    var value:test_hover_test031 = test_hover_test031(1)
    var a:int32 = value.add(1)
    var a1:int32 = value.add(1,2)
}
