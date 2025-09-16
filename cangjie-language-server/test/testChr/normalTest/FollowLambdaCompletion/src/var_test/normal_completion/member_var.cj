// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.var_test.normal_completion

class CuStomClass3 {
    let aa: Int64
    let bb: String
    let cc: ((String, Int64) -> Int64) -> Int64

    init(aa: Int64, bb: String, cc: ((String, Int64) -> Int64) -> Int64) {
        this.aa = aa
        this.bb = bb
        this.cc = cc
    }

    func func2() {
        cc
    }
}
