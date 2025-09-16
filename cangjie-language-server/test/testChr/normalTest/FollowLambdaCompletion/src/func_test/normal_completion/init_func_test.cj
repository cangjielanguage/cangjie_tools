// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.func_test.normal_completion

class CuStomClass1 {
    let aa: Int64
    let bb: String
    let cc: (Int64, Int64) -> Int64

    init(aa: Int64, bb: String, cc: (Int64, Int64) -> Int64) {
        this.aa = aa
        this.bb = bb
        this.cc = cc
    }
}

struct CustomStruct1 {
    var aa: Int64 = 10
    var bb: String = ""
    var cc: (Int64, String) -> String

    init(aa: Int64, bb: String, cc: (Int64, String) -> String) {
        this.aa = aa
        this.bb = bb
        this.cc = cc
    }
}

func init_test() {
    var aa = CustomClass1
    var bb = CustomStruct1
}
