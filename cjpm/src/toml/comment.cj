// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.collection.*

public struct Comment <: Collection<String> & ToString {
    let data: ArrayList<String>

    public init() {
        data = ArrayList<String>()
    }

    public init(cap: Int64) {
        data = ArrayList<String>(cap)
    }

    public init(elements: Collection<String>) {
        data = ArrayList<String>(elements)
    }

    public prop size: Int64 {
        get() {
            return this.data.size
        }
    }

    public func isEmpty(): Bool {
        return data.isEmpty()
    }

    public func append(element: String): Unit {
        this.data.add(element)
    }

    public func append(comment: Comment): Unit {
        this.data.add(all: comment.data)
    }

    public operator func [](index: Int64): String {
        return this.data[index]
    }

    public func iterator(): Iterator<String> {
        return this.data.iterator()
    }

    public func toString(): String {
        let sb = StringBuilder()
        for (i in 0..this.data.size) {
            sb.append("#")
            sb.append(data[i])
            if (i != data.size - 1) {
                sb.append("\n")
            }
        }
        return sb.toString()
    }

    public func clear(): Unit {
        this.data.clear()
    }

    public func clone(): Comment {
        return Comment(this.data)
    }
}
