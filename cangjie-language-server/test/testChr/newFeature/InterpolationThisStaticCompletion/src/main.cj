// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package interpolation_this_static

class Sample {
    public let id: Int64 = 1
    private static let STATIC_ONLY: Int64 = 3

    static func describe(): String {
        "${Sample.STATIC_ONLY}"
    }
}
