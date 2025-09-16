// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class fatherC {
    static private var staticPrivateFatherA = 1
    static public var staticPublicFatherB = 2
    static var staticPublicFatherB2 = 2
    static protected var staticProtectedFatherC =3
    static private func staticPrivateFatherFuncA(){}
    static public func staticPublicFatherFuncB(){}
    static func staticPublicFatherFuncB2(){}
    static protected func staticProtectedFatherFuncC(){}
    static mut prop fatherProp: Int64 {
         get() {
             print("get")
             return 1
         }
         set(value){
            print("set")
            let Cvar1 = value
         }
    }
}
class child3 <: fatherC {
    static public var child3C = 1
}
var ttt  = child3()
var ttt1 = child3.a
