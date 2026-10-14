// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package unusedDefaultParams

public func defaultReadsEarlierParam(id: String, name!: String = id) {
    println(name)
}

public func unusedLocalInDefault(id: String, name!: String = ({ =>
    let unusedValue = 1
    "aaa"
})()) {
    println(id + name)
}
