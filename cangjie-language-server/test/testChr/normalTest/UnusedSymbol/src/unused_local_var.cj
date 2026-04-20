// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// Test C1: Unused local variable - should be reported as unused
package testUnusedSymbol

func testUnusedLocalVar(): Unit {
    var unusedLocal: Int64 = 10
    var usedLocal: Int64 = 20
    println(usedLocal)
}
