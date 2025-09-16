// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test1111

public open class Rect {
    public init(w: Int64, h: Int64) {
    }
}

class Cu <: Rect {
    private var l: Int64
    init(w: Int64, h: Int64) {
        super(w, h)
        this.l = 1
    }
}
