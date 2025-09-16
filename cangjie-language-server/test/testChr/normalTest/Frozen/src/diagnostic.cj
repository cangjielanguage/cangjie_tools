// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjie1006

@Frozen
public func testFunc() {

}

class TestClass1 {
    @Frozen
    public prop a: Int64 {
        get() { 1 }
    }
 
    @Frozen
    public func getA() {
        return 1
    }
}

struct TestStruct {
    let a = 1

    @Frozen
    public func getA() {
        return a
    }
}

public enum TestEnum {
    | ENUM1
    | ENUM2

    @Frozen
    public func getV() {
        return 1
    }
}

interface TestInterface {
 
    @Frozen
    prop a: Int64 {
        get() { 1 }
    }
 
    @Frozen
    func getA() {
        return 1
    }
}

extend TestClass1 {
    @Frozen
    public prop b: Int64 {
        get() { 1 }
    }

    @Frozen
    public func getB() {
        return 1
    }
}
