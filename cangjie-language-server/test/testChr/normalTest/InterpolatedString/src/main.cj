// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

from std import collection.ArrayList

main(): Int64 {
    let array = ArrayList<Int64>()
    let v = "current phase:,plugin list size:${array.size}"
    for (a in array){
        print("********")
    }
    println("hello world")
    return 0
}
