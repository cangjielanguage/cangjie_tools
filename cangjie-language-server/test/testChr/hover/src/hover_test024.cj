// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
*test hover
*testpkg Num_hover
*/
trait Num_hover24 {
    /**
    *test hover
    *testpkg Num_hover plus
    */
    func plus24(a: This, b: This): This
}

append int32 with Num_hover24 {
    func plus24(a: int32, b: int32): int32 { a + b }
}
