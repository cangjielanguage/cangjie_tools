// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package FollowLambdaCompletion.func_test.normal_completion

class Rectangle {
    public Rectangle(let width: Int64, let height: Int64, let callback: (Int64) -> String) {}
}

struct Rectangle1 {
    public Rectangle1(let callback: (aa: Int64, bb: String) -> String) {}
}

func test_primary() {
    var aa = Rectangle
    var bb = Rectangle1
}
