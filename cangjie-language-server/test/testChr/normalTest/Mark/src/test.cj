// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

main(): Int64 {
    let f1: (l:Int64, r: ABC) -> Int64 = {l, r => l + r.abc}
    let c:(a:ABC, b:Int64) -> Int32
    return 0
}
class ABC {
    public var abc = 1
}
func printAdd(add: (a: ABC, b:Int64) -> Int64, a: ABC, b: Int64): Unit {
    println(add(a, b))
}



func add(a: ABC, b: Int64): Int64 {
    a.abc + b
}

func returnAdd(): (a: ABC, b: Int64) -> Int64 {
    add
}
