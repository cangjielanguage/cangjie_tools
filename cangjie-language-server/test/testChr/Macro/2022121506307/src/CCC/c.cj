// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package CCC

from std import ast.*
public macro MM2(attr: Tokens, input: Tokens): Tokens {
    var decl = (parseDecl(attr) as ClassDecl).getOrThrow()
    var dec2 = (parseDecl(input) as ClassDecl).getOrThrow()
    return quote(class CMMB {$(dec2.body.decls)};)+quote(class CMM1 {$(decl.body.decls) var b = 1; public func hh1(){}
    })
}

public macro BB3(attr:Tokens,input: Tokens): Tokens {
    return quote(public enum EEB2{
        |ABC
        public func ab2(x:Int64){} 
        
    })
}
