// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.func_test.dot_completion

class CuStomClass2 {
    let aa: Int64 = 10
    let bb: String = ""

    func member_func1(arg1: Int64, callback: (aa: Int64, bb: String) -> String): String {
        ""
    }

    func member_func2(callback: () -> String): Int64 {
        0
    }
}

struct CuStomStruct2 {
    let aa: Int64 = 10
    let bb: String = ""

    func member_func1(callback: (Int64, String, Unit) -> String): String {
        ""
    }
}

func test_dot_completion() {
    var aa = CuStomClass2()
    aa.member_func1
    aa.member_func2
    var bb = CuStomStruct2()
    bb.member_func1
}
