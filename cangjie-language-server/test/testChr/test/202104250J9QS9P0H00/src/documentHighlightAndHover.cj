// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.collection.*

// List关键字、List类型变量应用于class、enum内部
class DataClass {
    var a: ArrayList<Int64>
    init(x: ArrayList<Int64>) {
        a = x
    }
    func foo(): ArrayList<Int64> {
        var a = [1, 2, 3]
        return a
    }

    static func bar(x: ArrayList<Int64>): ArrayList<Int64> {
        return x
    }
}
enum Time1 { Year(ArrayList<Int32>)}
enum Time2 { Year(Set<Int32>)}
enum Time3 { Year(Map<Int32,Int32>)}
