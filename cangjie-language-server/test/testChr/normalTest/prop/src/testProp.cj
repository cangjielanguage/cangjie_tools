// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface TestInterfaceProp{
    static prop testProp:Int64 {
        get(){
            return 0
        }
    }
}
struct TestRecordProp<: TestInterfaceProp {
    static prop testProp:Int64 {
        get(){
            return 0
        }
    }
}
class TestClassProp <: TestInterfaceProp{
    static prop testProp:Int64 {
        get(){
            return 0
        }
    }
}

class Base {
    static prop staticProp:Int64 {
        get(){
            return 0
        }
    }

    open prop overrideProp:Int64
    {
        get(){
            return 0
        }
    }
}

open class Derived <: Base {
    redef static prop staticProp:Int64 {
        get(){
            return 0
        }
    }

    override open prop overrideProp:Int64 {
        get(){
            return 0
        }
    }
}

open class Son <: Derived {
    redef static prop staticProp:Int64 {
        get(){
            return 0
        }
    }

    override prop overrideProp:Int64 {
        get(){
            return 0
        }
    }
}
