// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro  package mmm
from std import ast.*

public macro M3(input: Tokens): Tokens {
    var decl = parseDecl(input)
    return quote(class $(decl.identifier)
    { 
        static var aa = $(decl.identifier)(2);
        init(a:Int64){}
        init(){}
    };)
}

