// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.toml

enum ITEMTYPE <: Equatable<ITEMTYPE> & ToString {
    | ERROR
    | NIL
    | EOF
    | TEXT
    | STRING
    | RAWSTRING
    | MULTILINESTRING
    | RAWMULTILINESTRING
    | BOOL
    | INTEGER
    | FLOAT
    | DATETIME
    | LOCALTIME
    | ARRAY
    | ARRAYEND
    | TABLESTART
    | TABLEEND
    | ARRAYTABLESTART
    | ARRAYTABLEEND
    | KEYSTART
    | KEYEND
    | COMMENTSTART
    | INLINETABLESTART
    | INLINETABLEEND

    public func value(): Int64 {
        match (this) {
            case ERROR => return 0
            case NIL => return 1
            case EOF => return 2
            case TEXT => return 3
            case STRING => return 4
            case RAWSTRING => return 5
            case MULTILINESTRING => return 6
            case RAWMULTILINESTRING => return 7
            case BOOL => return 8
            case INTEGER => return 9
            case FLOAT => return 10
            case DATETIME => return 11
            case LOCALTIME => 23
            case ARRAY => return 12
            case ARRAYEND => return 13
            case TABLESTART => return 14
            case TABLEEND => return 15
            case ARRAYTABLESTART => 16
            case ARRAYTABLEEND => 17
            case KEYSTART => return 18
            case KEYEND => return 19
            case COMMENTSTART => return 20
            case INLINETABLESTART => return 21
            case INLINETABLEEND => return 22
        }
    }

    public func toString(): String {
        match (this) {
            case ERROR => return "Error"
            case NIL => return "NIL"
            case EOF => return "EOF"
            case TEXT => return "Text"
            case STRING => return "String"
            case RAWSTRING => return "String"
            case MULTILINESTRING => return "String"
            case RAWMULTILINESTRING => return "String"
            case BOOL => return "Bool"
            case INTEGER => return "Integer"
            case FLOAT => return "Float"
            case DATETIME => return "DateTime"
            case LOCALTIME => "LocalTime"
            case ARRAY => return "Array"
            case ARRAYEND => return "ArrayEnd"
            case TABLESTART => return "TableStart"
            case TABLEEND => return "TableEnd"
            case ARRAYTABLESTART => return "ArrayTableStart"
            case ARRAYTABLEEND => return "ArrayTableEnd"
            case KEYSTART => return "KeyStart"
            case KEYEND => return "KeyEnd"
            case COMMENTSTART => return "CommentStart"
            case INLINETABLESTART => return "InlineTableStart"
            case INLINETABLEEND => return "InlineTableEnd"
        }
    }

    public operator func ==(rhs: ITEMTYPE): Bool {
        return this.value() == rhs.value()
    }

    public operator func !=(rhs: ITEMTYPE): Bool {
        return this.value() != rhs.value()
    }
}

struct Item <: ToString {
    let typ: ITEMTYPE
    let val: String
    let err: Option<Exception>
    let pos: Position

    public init(typ: ITEMTYPE, pos: Position, val: String, err: Option<Exception>) {
        this.typ = typ
        this.val = val
        this.pos = pos
        this.err = err
    }

    public func toString(): String {
        return "(${typ}, ${val}:${pos})"
    }
}
