// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import AAA.*

@A
func test1() {
    var a = 10;
    var b = a
    var c = a
    println("test1");
}

@A
open class TestClass {
    public var aa = 10;

    public init() {
        
    }

    func testFunc() {
        println("testFunc");
    }

    func testFunc2(): Int32 {
        testFunc()
        test1()
        println("testFunc2");
        0
    }
}

@A
public struct TestStruct {
    public TestStruct() {}

    func testStructFunc() {
        try {
            @B(c = 1)
        } catch (e: Exception) {}
        println("testStructFunc");
    }

    func testStructFunc2(k: Int32): Int32 {
        println("testStructFunc2 : ${k}");
        if (k < 10) {
            var s = k + 1
            testStructFunc2(s)
        }
        return 0
    }
}

@A
@Z
func c() {
    var a = TestClass();
    var b = a.aa;
}

@D[var myVal = 10]
func useOutMacro() {
    println("useOutMacro")
    declInMacro();
    println(myVal)
}
