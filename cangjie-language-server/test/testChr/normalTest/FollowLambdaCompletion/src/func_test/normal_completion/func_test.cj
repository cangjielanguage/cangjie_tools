// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.func_test.normal_completion

func func1(arg1: Int64, callback: (Int64)): Int64 {
    0
}

func func2(arg1: Int64, callback: (aaa: Int64, sss: String) -> String): Int64 {
    0
}

func func3(arg1: Int64, callback: (Int64, String) -> String): Int64 {
    0
}

func func4(arg1: Int64, callback: () -> String): Int64 {
    0
}

func func5(callback: (aaa: Int64) -> String): Int64 {
    0
}

func func6(arg1: Int64, cc: (aaa: Int64, callback1: (bbb: Int64, ccc: (Int64, String)) -> String) -> String): Int64 {
    0
}

func func_test() {
    func2
    func3
    func4
    func5
    func6
}
