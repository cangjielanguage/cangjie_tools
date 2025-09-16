// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import default.pkgA.*

public class public_cur_class{
    init() {}
    init(public_cur_classA: Int64) {}
    public_cur_class(public_cur_classB!:Float32,public_cur_classS!:String="3"){}
    func public_cur_class_func(public_cur_classA: Int64) {}
    func public_cur_class_func(public_cur_classB1!:Float32,public_cur_classS1!:String="3") {}
    func public_cur_class_func(public_cur_classA2: Int64,public_cur_classB2!:Float32,public_cur_classS2!:String="3") {}
}

extend public_cur_class{
     func public_cur_class_func() {}
}

class cur_class{
    init(a: Int64) {}
    cur_class(b!:Float32,s!:String = "3"){}
    func cur_class_func(cur_class_funcA: Int64) {}
    func cur_class_func(cur_class_funcb1!:Float32,cur_class_funcs2!:String="3") {}
    func cur_class_func(cur_class_funca2: Int64,cur_class_funcb2!:Float32,cur_class_funcs2!:String="3") {}
}


public enum public_cur_enum{
    enumA|enumB|enumC|enumD
    func public_cur_enum_func(public_cur_enum_funca: Int64) {}
    func public_cur_enum_func(public_cur_enum_funcb1!:Float32,public_cur_enum_funcs1!:String="3") {}
    func public_cur_enum_func(public_cur_enum_funca2: Int64,public_cur_enum_funcb2!:Float32,public_cur_enum_funcs2!:String="3") {}
}

public struct public_cur_struct{
    init(a: Int64) {}
    public_cur_struct(b!:Float32,s!:String="3"){}
    func public_cur_struct_func(public_cur_struct_funca: Int64) {}
    func public_cur_struct_func(public_cur_struct_funcb1!:Float32,public_cur_struct_funcs1!:String="3") {}
    func public_cur_struct_func(public_cur_struct_funca2: Int64,public_cur_struct_funcs2!:String="3",public_cur_struct_funcb2!:Float32) {}
}

public func public_cur_func() {}
public func public_cur_func(public_cur_funca: Int64) {}
public func public_cur_func(public_cur_funcb1!:Float32,public_cur_funcs1!:String="3") {}
public func public_cur_func(public_cur_funca2: Int64,public_cur_funcb2!:Float32,public_cur_funcs2!:String="3") {}

let a1 = public_cur_func(,)
let a2 = public_cur_func(public_cur_funcb1,)
let a3 = public_cur_func(public_cur_funcb1,)

let a4 = myfunc(,)
let a5 = pkgA.public_pkgA_class().public_pkgA_class_func(,)
let a6 = public_pkgA_class().public_pkgA_class_func(,)

let a4 = public_pkgA_struct(,)
let a5 = pkgA.public_pkgA_struct(,)
let a6 = pkgA.public_pkgA_struct(1).public_pkgA_struct_func(,)
