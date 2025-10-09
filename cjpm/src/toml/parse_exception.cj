// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

class ParseException <: Exception {
    public init(message: String) {
        super(message)
    }

    public init(message: String, pos: Position) {
        super("toml: line ${pos.line}: ${message}")
    }

    public init(message: String, pos: Position, lastKey: String) {
        super("toml: line ${pos.line} (last key ${lastKey}): ${message}")
    }

    protected override func getClassName(): String {
        return "ParseException"
    }
}
