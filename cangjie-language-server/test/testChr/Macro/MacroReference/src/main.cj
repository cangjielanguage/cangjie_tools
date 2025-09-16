// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// func namedPara(param!:C<ParamType>) {}


main() {
    test1()

    var testClass = TestClass()
    testClass.testFunc()
    testClass.testFunc2()

    var testStruct = TestStruct()
    testStruct.testStructFunc()
    testStruct.testStructFunc2(0)

    declInMacro();
    useOutMacro();

    var x: Int32 = 1
    var f: Float64 = 1.1
    var z: Bool = true
    return 0
}

