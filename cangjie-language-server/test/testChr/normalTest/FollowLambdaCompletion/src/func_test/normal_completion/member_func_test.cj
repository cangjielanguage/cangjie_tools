// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.func_test.normal_completion

interface CustomInterface {
    func inter_func(aa: (String, Int64) -> Int64): Int64
}

class CuStomClass <: CustomInterface {
    let aa: Int64 = 10
    let bb: String = ""

    public func inter_func(aa: (String, Int64) -> Int64): Int64 {
        0
    }

    func member_func1(arg1: Int64, callback: (aa: Int64, bb: String) -> String): String {
        ""
    }

    func member_func2() {
        member_func1
        inter_func
    }
}

struct CustomStruct {
    var aa: Int64 = 10
    var bb: String = ""

    public func member_func1(aa: Int64, bb: (arg1: Int64, arg2: String) -> String) {
        
    }

    func member_func2() {
        member_func1
    }
}
