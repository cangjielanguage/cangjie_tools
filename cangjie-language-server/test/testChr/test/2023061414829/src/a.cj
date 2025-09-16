// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func foo(x!: Array<Int64> = [1,2,3,4]): Array<Int64> {
    var a = x
    return a
}

var bara: (Array<Int64>)->Array<Int64> = {x:Array<Int64> => x}
var intArray:Array<Int64> = [1,2,3,4]
