// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/17
 */
package ohos_app_cangjie_entry.ExtractedVariable

func test01() {
    var aa = 10
    if (aa > 0) {
        var cc = 10
    } else {
        aa++
    }
}

func test02() {
    var aa = 10
    var bb = 10
    var cc = 10
    var dd = 10
    var ee = aa + bb + dd + cc
}
