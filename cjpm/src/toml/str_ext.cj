// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.collection.*

struct Span {
    var start: Int64
    var end: Int64
    public init(start: Int64, end: Int64) {
        this.start = start
        this.end = end
    }
}

extend String {
    func split(f: (Rune) -> Bool): Array<String> {
        let spans = ArrayList<Span>(32)
        var start = -1
        let rarr = this.toRuneArray()
        for (end in 0..rarr.size) {
            let c = rarr[end]
            if (f(c)) {
                if (start >= 0) {
                    spans.add(Span(start, end))
                    start = !start
                }
            } else {
                if (start < 0) {
                    start = end
                }
            }
        }
        if (start >= 0) {
            spans.add(Span(start, this.size))
        }

        let a = Array<String>(spans.size, repeat: unsafe { zeroValue<String>() })
        for (i in 0..spans.size) {
            let span = spans[i]
            a[i] = this[span.start..span.end]
        }
        return a
    }

    func stripLeft(prefix: String): String {
        var s = this
        for (i in 0..this.size) {
            if (!prefix.contains(s[i..])) {
                break
            }
            s = s[1..]
        }
        return s
    }

    func stripRight(sufix: String): String {
        var s = this
        while (s.size > 0) {
            if (!sufix.contains(s[s.size - 1..])) {
                break
            }
            s = s[..s.size - 1]
        }
        return s
    }
}
