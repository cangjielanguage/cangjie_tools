// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// Test A7: Mixed unused symbols - multiple types should be reported as unused
package testUnusedSymbol

private func unusedFunc1(): Unit {}
private func unusedFunc2(): Unit {}

func testUseUnusedFunc1(): Unit {
    unusedFunc1()
}

private let unusedVar1: Int64 = 1
private let unusedVar2: String = "test"

private class UnusedClass1 {}
private class UnusedClass2 {}

private struct UnusedStruct1 {}
private struct UnusedStruct2 {}

private enum UnusedEnum {
    | Case1
    | Case2
}

private interface UnusedInterface {
    func method(): Int64
}
