// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package sub

from std import ast.*

public macro P1(input:Tokens){
    input
}

public macro P2(input: Tokens): Tokens {
    return quote(public class PC1{
        public var ab =1
        func gg1(x:Int64){}
    })
}

