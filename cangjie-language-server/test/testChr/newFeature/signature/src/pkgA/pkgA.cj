// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkgA
public import default.pkgB.aaaa.*
public import default.pkgB.public_pkgB_func as myfunc

let aa2 = myfunc(,)
let aaa = aaaa.enumFunc(,)

public struct xxxx{
    let a = 1
}
func dd(a:Int64){
}
let c = pkgA.dd(1)
public class public_pkgA_class{
    init() {}
    init(public_pkgA_classA: Int64) {}
    public_pkgA_class(public_pkgA_classB!:Float32,public_pkgA_classS!:String="3"){}
    func public_pkgA_class_func(public_pkgA_classA: Int64) {}
    func public_pkgA_class_func(public_pkgA_classB1!:Float32,public_pkgA_classS1!:String="3") {}
    func public_pkgA_class_func(public_pkgA_classA2: Int64,public_pkgA_classB2!:Float32,public_pkgA_classS2!:String="3") {}
}

extend public_pkgA_class{
     func public_pkgA_class_func() {}
}

class pkgA_class{
    init(a: Int64) {}
    pkgA_class(b!:Float32,s!:String = "3"){}
    func pkgA_class_func(pkgA_class_funcA: Int64) {}
    func pkgA_class_func(pkgA_class_funcb1!:Float32,pkgA_class_funcs2!:String="3") {}
    func pkgA_class_func(pkgA_class_funca2: Int64,pkgA_class_funcb2!:Float32,pkgA_class_funcs2!:String="3") {}
}


public enum public_pkgA_enum{
    enumA|enumB|enumC|enumD
    func public_pkgA_enum_func(public_pkgA_enum_funca: Int64) {}
    func public_pkgA_enum_func(public_pkgA_enum_funcb1!:Float32,public_pkgA_enum_funcs1!:String="3") {}
    func public_pkgA_enum_func(public_pkgA_enum_funca2: Int64,public_pkgA_enum_funcb2!:Float32,public_pkgA_enum_funcs2!:String="3") {}
}

public struct public_pkgA_struct{
    init(a: Int64) {}
    public_pkgA_struct(b!:Float32,s!:String="3"){}
    func public_pkgA_struct_func(public_pkgA_struct_funca: Int64) {}
    func public_pkgA_struct_func(public_pkgA_struct_funcb1!:Float32,public_pkgA_struct_funcs1!:String="3") {}
    func public_pkgA_struct_func(public_pkgA_struct_funca2: Int64,public_pkgA_struct_funcs2!:String="3",public_pkgA_struct_funcb2!:Float32) {}
}

public func public_pkgA_func() {}
public func public_pkgA_func(public_pkgA_funca: Int64) {}
public func public_pkgA_func(public_pkgA_funcb1!:Float32,public_pkgA_funcs1!:String="3") {}
public func public_pkgA_func(public_pkgA_funca2: Int64,public_pkgA_funcb2!:Float32,public_pkgA_funcs2!:String="3") {}

