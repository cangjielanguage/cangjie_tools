// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.var_test.normal_completion

func local_var_test() {
    var aaa: (String, String) -> String
    aaa
    var bbb: (String, String, (Int64) -> Int64) -> String
    bbb
}
