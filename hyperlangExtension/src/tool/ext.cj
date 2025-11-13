// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.collection.ArrayList

extend<T> Array<T> {
    public func map<R>(mapper: (e: T, i: Int64) -> R): Array<R> {
        let list = ArrayList<R>()
        for (i in 0..this.size) {
            list.add(mapper(this[i], i))
        }
        return list.toArray()
    }
}