// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.testImport.pkg

var s = yourStruct(1, 2)

public struct yourStruct{
    public var a:Int64
    public var b:Int64
    public init(a:Int64, b:Int64){
        this.a = a
        this.b = b
        println("hello word\n")
        print('\n')
    }
}

public open class yourClass {
    public yourClass(a:Int64){}
}
