// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

// EXEC: %compiler %cmp_opt %f -o %output
// EXEC: %run %run_opt %output %run_args

// Set关键字、Set类型变量作为函数入参/返回值/函数默认值
func foo(x!: Set<Int16> = #{1,2,3}): Set<Int16> {
    var a = x
    return a
}
var bar11111: (Set<Int64>)->Set<Int64> = {ttt: Set<Int64> => ttt}
func main(): Int64 {
    var intSet: Set<Int64> = #{xxxx1111,2,3}
    foo()
    bar11111(intSet)
    return 0
}
