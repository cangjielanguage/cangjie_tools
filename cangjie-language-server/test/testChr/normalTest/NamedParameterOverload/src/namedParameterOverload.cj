// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.collection.ArrayList

func test() {
    let list = ArrayList<Int64> ()
    list.add(0)
    list.add(1)

    let list2 = ArrayList<UInt32> ()
    list2.add(0)

    let li = [2, 3]
    list.add(all: li)
    list.add(all: li)
}
