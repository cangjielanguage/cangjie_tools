// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testPkg
extend String{
    func testPkgExtendFunc():Bool {
        return this.isEmpty();
    }
}

func testPkgTopLevelFunc(a:Int64, b:Int64):Int64 {
    return a + b
}

enum testPkgEnum {
    BenzEnum |
    AudiEnum(String)

    func testPkgEnumFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testPkgEnumProp:String {
        get() {
            return testPkgEnumProp;
        }
    }   
}

struct testPkgRecord {
    var structVariables:String;

    testPkgRecord(str:String) {
        structVariables = str;
    }
    init() {
        structVariables = ""
    }
    func testPkgRecordFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testPkgRecordProp:String {
        get() {
            return testPkgRecordProp;
        }
    }   
}

class testPkgClass{
    var testPkgClassString:String;
    
    testPkgClass(str:String) {
        testPkgClassString = str;
    }

    init() {
        testPkgClassString = ""
    }

    func testPkgClassFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testPkgClassProp:String {
        get() {
            return testPkgClassProp;
        }
    }   
}

interface testPkgInterface{
    func testPkgInterfaceFunc(a:Int64, b:Int64):Int64 {
        return a + b
    }

    prop testPkgInterfaceProp:String {
        get() {
            return testPkgInterfaceProp;
        }
    }   
}

external var testPkgToplevelVarable: (Int64, Int64) -> Int64 = {a, b => return a + b}

type testPkgTypealiasOfTuple = Array<Array<Int64>> * Array<Array<Int64>>
type testPkgTypealiasOfLambda = (Int64, Int64) -> Int64;

