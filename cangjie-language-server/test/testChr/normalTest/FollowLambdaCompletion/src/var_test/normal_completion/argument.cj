// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.var_test.normal_completion

func func3(arg1: Int64, cc: (aaa: Int64, callback1: (bbb: Int64, ccc: (Int64, String)) -> String) -> String): Int64 {
    cc
    0
}
