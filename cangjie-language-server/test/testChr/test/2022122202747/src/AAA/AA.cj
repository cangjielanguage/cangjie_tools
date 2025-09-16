// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

macro package AAA 

from std import ast.*


public macro A1(input:Tokens){
    input
}

public macro A2(input:Tokens){
    return quote($input)
}

public macro A3(input:Tokens){
    return input
}
