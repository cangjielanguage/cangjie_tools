// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package interpolation_this

class Sample {
    public let id: Int64 = 1
    private let label: String = "sample"
    private let value: Int64 = 2
    private static let STATIC_ONLY: Int64 = 3

    func describe(): String {
        "${this.label}#${this.id}:${this.value}"
    }
}
