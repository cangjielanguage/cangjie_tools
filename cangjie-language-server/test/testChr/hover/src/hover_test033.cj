// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
*test hover
*test_hover_test033
*/
open class test_hover_test033 {
    var aaa:int32 = 1
    /**
    *test hover
    *test_hover_test033 init(a:int32)
    */
    init(a:int32) {
        aaa = a
    }
    /**
    *test hover
    *test_hover_test033 add1(a:int32, b:int32)
    */
    func add1(a:int32, b:int32):int32 {
        return a + b
    }
}

class test_hover_test033_s <: test_hover_test033 {
    /**
    *test hover
    *test_hover_test033_s a
    */
    var a:int32 = 1
    init(a:int32) {
        super(1)
        this.a = a
    }
}
