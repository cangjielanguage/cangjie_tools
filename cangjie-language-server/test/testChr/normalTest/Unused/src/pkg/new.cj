// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package test0512.pkg
import std.ast.*

public macro Demo(input:Tokens):Tokens{
    parseDecl(input).dump()  //使用dump函数打印语法树
    return input
}


