// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package code.callHierarchy2
import code.callHierarchy1.*
//包外
func test12():Unit{
let sum : (Int64,Int64)->Int64 = {a,b =>a+b}/*包外*/
sum(1,2)//包外
let a00 = A0()//包外
let a01 = A0()//包外
let a10 = A1()//包外
let a11 = A1()//包外
let a20 = A2()//包外
let a21 = A2()//包外
let a22 = a20+a21//包外
let human1 = human()//包外
let People1 = People(1,2)//包外
println("今天是周五")//包外
}
