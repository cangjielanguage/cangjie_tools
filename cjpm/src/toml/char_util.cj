// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

let TOML_EOF = r'\u{00}'
let MAX_CHAR = 0x0010FFFFu32 // Maximum valid Unicode code point.
let SURROGATE_MIN = 0xD800u32
let SURROGATE_MAX = 0xDFFFu32

func validChar(c: Rune) {
    let u = UInt32(c)
    match {
        case 0 <= u && u < SURROGATE_MIN => return true
        case SURROGATE_MAX < u && u <= MAX_CHAR => return true
        case _ => ()
    }
    return false
}

func charOrEOF(c: Rune): String {
    if (c == TOML_EOF) {
        return "end of file"
    }
    return "'${c}'"
}

func isWhitespace(c: Rune): Bool {
    return c == r'\t' || c == r' '
}

func isNL(c: Rune): Bool {
    return c == r'\n' || c == r'\r'
}

func isControl(c: Rune): Bool { // Control characters except \t, \r, \n
    match (c) {
        case r'\t' | r'\r' | r'\n' => return false
        case _ => return (c >= Rune(0x00) && c <= Rune(0x1f)) || c == Rune(0x7f)
    }
}

func isDigit(c: Rune): Bool {
    return c >= r'0' && c <= r'9'
}

func isBinary(c: Rune): Bool {
    return c == r'0' || c == r'1'
}

func isOctal(c: Rune): Bool {
    return c >= r'0' && c <= r'7'
}

func isHexadecimal(c: Rune): Bool {
    return (c >= r'0' && c <= r'9') || (c >= r'a' && c <= r'f') || (c >= r'A' && c <= r'F')
}

func isBareKeyChar(c: Rune): Bool {
    return (c >= r'A' && c <= r'Z') || (c >= r'a' && c <= r'z') || (c >= r'0' && c <= r'9') || c == r'_' || c == r'-'
}

func isLetter(c: Rune): Bool {
    return c.isAsciiLetter()
}
