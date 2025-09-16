// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import default.testList.*
import std.collection.*
func testPkg(){
    for(num in externalList) {
        print("${num}")
    }
}

let emptyList : ArrayList<Int64> = ArrayList<Int64>()
let list0 = [1, 2, 3, emptyList, 3, 2, 1]
