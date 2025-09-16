// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjie1006

@Frozen
class TestClass2 {
    @Frozen
    public var a = 1
}

@Frozen
struct TestStruct2 {
    @Frozen
    public var a = 1
}

@Frozen
enum TestEnum2 {
    | ENUM1
    | ENUM2
}

@Frozen
interface TestInterface2 {}

@Frozen
extend TestClass2 {} 

main(): Int64 {
    @Frozen
    func test() {
        return 0
    }
    return test()
}
