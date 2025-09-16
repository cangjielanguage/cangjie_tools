// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package default.define
import std.ast.*

public macro X12(attr:Tokens,input:Tokens){
    return attr + quote(-) + input
}

public macro Y1232(input:Tokens){
    return input + quote(+) + input
}

public macro Z12345(input:Tokens, attr:Tokens){
    return input + quote(+) + attr
}

public macro Test12(attr:Tokens,input:Tokens){
    return attr + quote(-) + input
}


public macro T1(input:Tokens) {
    return input
}
