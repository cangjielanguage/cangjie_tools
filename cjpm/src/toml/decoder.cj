// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.io.*

public class Decoder {
    let r: InputStream

    public init(r: InputStream) {
        this.r = r
    }

    public func decode(): TomlObject {
        let sr = StringReader(r)
        let data = sr.readToEnd()
        let p = parse(data)
        return p.mapping
    }
}
