// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

import std.io.*

public class Decoder {
    let stream: ?InputStream
    let data: ?Array<Byte>

    // Keep stream consumption and I/O failures in decode(), matching the original API behavior.
    public init(r: InputStream) {
        this.stream = r
        this.data = None
    }

    // File-based callers should use this overload with the exact bytes returned by File.readFrom.
    public init(data: Array<Byte>) {
        this.stream = None
        this.data = data
    }

    public func decode(): TomlObject {
        let input = match (data) {
            case Some(bytes) => bytes
            case None => readStream()
        }
        let p = parse(input)
        return p.mapping
    }

    private func readStream(): Array<Byte> {
        let buffer = ByteBuffer()
        match (stream) {
            case Some(input) => copy(input, to: buffer)
            case None => throw Exception("decoder has no input")
        }
        return buffer.bytes()
    }
}
