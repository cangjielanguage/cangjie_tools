// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package code.callHierarchy
import logging.job.*
//来自模块外
func test11():Unit{
let sum : (Int64,Int64)->Int64 = {a,b =>a+b}/*来自模块外*/
sum(1,2)//来自模块外
let a00 = A0()//来自模块外
let a01 = A0()//来自模块外
let a10 = A1()//来自模块外
let a11 = A1()//来自模块外
let a20 = A2()//来自模块外
let a21 = A2()//来自模块外
let a22 = a20+a21//来自模块外
let human1 = human()//来自模块外
let People1 = People(1,2)//来自模块外
println("今天是周五")//来自模块外
}
