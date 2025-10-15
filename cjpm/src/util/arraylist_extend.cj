// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.util

import std.collection.*

public interface ArrayListExtends<T> {
    func addIfSome(value: Option<T>): Unit
    func addIfSome(all!: Iterable<Option<T>>): Unit
}

extend <T> ArrayList<T> <: ArrayListExtends<T> { 
    public func addIfSome(element: Option<T>): Unit {
        if (let Some(v) <- element) {
            this.add(v)
        }
    }

    public func addIfSome(all!: Iterable<Option<T>>): Unit {
        let result = all |> filterMap { it: Option<T> => it } |> collectArray
        if (result.size != 0) {
            this.add(all: result)
        }
    }
}
