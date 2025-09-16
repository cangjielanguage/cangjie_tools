// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class LongNameClassA{
    public init(x:Int64){}
    var num=8
}
type B1B = LongNameClassA
type B2B = B1B

func test39(){
    var t1 = B1
    var t2 = B2B(6)
}
