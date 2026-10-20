// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

main(): Int64 {
    var 测表aa = [1, 2, 3, 4]
    if (测表aa.size > 0) {
        for (i in 0..测表aa.size) {
            println(测表aa[i])
        }
    }
    while (测表aa.size > 0) {
        break
    }
    let pair = (测表aa, 测表aa.size)
    let wrapped = (测表aa)
    let nested = ((测表aa))
    let sizes = (测表aa.size, 测表aa.size)
    println("中文😀${测表aa.size}")
    if (true) {
        let 测表aa = [5, 6]
        println(测表aa.size)
    }
    var ascii = [1, 2, 3, 4]
    if (ascii.size > 0) {
        for (i in 0..ascii.size) {
            println(ascii[i])
        }
    }
    while (ascii.size > 0) {
        break
    }
    let asciiPair = (ascii, ascii.size)
    let asciiWrapped = (ascii)
    let asciiNested = ((ascii))
    return 0
}
