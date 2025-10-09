// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.collection.*

struct Key <: Collection<String> & ToString {
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

    public func append(key: Key): Unit {
        this.data.add(all: key.data)
    }

    public func cloneAppend(elements: Collection<String>): Key {
        let newKey = Key(this.data)
        newKey.data.add(all: elements)
        return newKey
    }

    public operator func [](index: Int64): String {
        return this.data[index]
    }

    public operator func [](range: Range<Int64>): ArrayList<String> {
        return this.data[range]
    }

    public func iterator(): Iterator<String> {
        return this.data.iterator()
    }

    public func toString(): String {
        let ss = Array<String>(data.size, repeat: "")
        for (i in 0..data.size) {
            ss[i] = maybeQuoted(i)
        }
        return String.join(ss, delimiter: ".")
    }

    public func maybeQuoted(i: Int64): String {
        if (data[i] == "") {
            return "\"\""
        }
        let runes = data[i].runes()
        for (c in runes) {
            if (!isBareKeyChar(c)) {
                return "\"" + quotedReplace(data[i]) + "\""
            }
        }
        return data[i]
    }

    public func add(piece: String): Key {
        let newKey = Key(data)
        newKey.append(piece)
        return newKey
    }

    public func clone(): Key {
        let newKey = Key(data)
        return newKey
    }
}
