// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.var_test.dot_completion

class CuStomClass2 {
    let aa: Int64 = 10
    let bb: (String, (String) -> String) -> String
}

struct CuStomStruct2 {
    let aa: Int64 = 10
    let bb: ((String) -> String) -> Int64
}

func test_dot_completion() {
    var aa = CuStomClass2()
    aa.bb
    var bb = CuStomStruct2()
    bb.bb
}
