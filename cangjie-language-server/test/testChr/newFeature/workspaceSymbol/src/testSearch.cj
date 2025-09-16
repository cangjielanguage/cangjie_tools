// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

extend String{
    func testExtendFunc():Bool {
        return this.isEmpty();
    }
}

func testTopLevelFunc(a:Int64, b:Int64):Int64 {
    return a + b
}

enum testEnum {
    Benz |
    Audi(String)

    func testEnumFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testEnumProp:String {
        get() {
            return testEnumProp;
        }
    }   
}

struct testRecord {
    var structVariables:String;

    testRecord(str:String) {
        structVariables = str;
    }
    init() {
        structVariables = ""
    }
    func testRecordFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testRecordProp:String {
        get() {
            return testRecordProp;
        }
    }   
}

class testClass{
    var testClassString:String;
    
    testClass(str:String) {
        testClassString = str;
    }

    init() {
        testClassString = ""
    }
    
    func testClassFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testClassProp:String {
        get() {
            return testClassProp;
        }
    }   
}

interface testInterface{
    func testInterfaceFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testInterfaceProp:String {
        get() {
            return testInterfaceProp;
        }
    }   
}


