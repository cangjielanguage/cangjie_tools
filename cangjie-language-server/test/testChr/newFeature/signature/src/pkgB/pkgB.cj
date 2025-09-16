// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkgB

public class public_pkgB_class{
    init(public_pkgB_classb: Int64) {}
    init() {}
    public_pkgB_class(public_pkgB_classssss!:Float32,public_pkgB_classdddd!:String="3"){}
    func public_pkgB_class_func(public_pkgB_class_funca: Int64) {}
    func public_pkgB_class_func(public_pkgB_class_funcb1!:Float32,public_pkgB_class_funcs1!:String="3") {}
    func public_pkgB_class_func(public_pkgB_class_funca2: Int64,public_pkgB_class_funcb2!:Float32,public_pkgB_class_funcs2!:String="3") {}
}

extend public_pkgB_class{
     func public_pkgB_class_func() {}
}

let a = public_pkgB_class()
class pkgB_class{
    init(a: Int64) {}
    pkgB_class(b!:Float32,s!:String="3"){}
    func pkgB_class_func(pkgB_class_funca: Int64) {}
    func pkgB_class_func(pkgB_class_funcb1!:Float32,pkgB_class_funcs1!:String="3") {}
    func pkgB_class_func(pkgB_class_funca2: Int64,pkgB_class_funcb2!:Float32,pkgB_class_funcs2!:String="3") {}
}


public enum public_pkgB_enum{
    enumA|enumB|enumC|enumD
    func public_pkgB_enum_func(public_pkgB_enum_funca0: Int64) {}
    func public_pkgB_enum_func(public_pkgB_enum_funcb1!:Float32,public_pkgB_enum_funcs1!:String="3") {}
    func public_pkgB_enum_func(public_pkgB_enum_funca2: Int64,public_pkgB_enum_funcb2!:Float32,public_pkgB_enum_funcs2!:String="3") {}
}

public struct public_pkgB_struct{
    init(a: Int64) {}
    public_pkgB_struct(b!:Float32,s!:String="3"){}
    func public_pkgB_struct_func(public_pkgB_struct_funca: Int64) {}
    func public_pkgB_struct_func(public_pkgB_struct_funcb!:Float32,public_pkgB_struct_funcs!:String="3") {}
    func public_pkgB_struct_func(public_pkgB_struct_funca: Int64,public_pkgB_struct_funcb!:Float32,public_pkgB_struct_funcs!:String="3") {}
}

public func public_pkgB_func() {}
public func public_pkgB_func(public_pkgB_funca: Int64) {}
public func public_pkgB_func(public_pkgB_funcb1!:Float32,public_pkgB_funcs1!:String="3") {}
public func public_pkgB_func(public_pkgB_funca2: Int64,public_pkgB_funcb2!:Float32,public_pkgB_funcs2!:String="3") {}
