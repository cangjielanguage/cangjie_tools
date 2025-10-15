// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry

let r1 = { a: Int64, b: Int64 => a + b }(1, 2) // r1 = 3,应该选中{}里面内容提取方法
let r2 = { => 123 }()

func test() {
    var aa = 10
    if (aa > 0 && aa + 10 > 20) {

    } else if (aa < 0) {
            aa > 0
        && aa + 10 > 20
    } else {
        aa++
    }
    aa > 0
        && aa + 10 > 20

    do {
        var ee = "xx"
        var ff = ee + "xx"
    } while (aa + 100 > 10)
}