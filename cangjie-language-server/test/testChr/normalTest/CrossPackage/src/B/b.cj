// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.B

import default.A.*

// 一般一级跨包继承场景  Scenarios1
public open class BClass <: AInterface {
    public open func funcAold(): Unit {
        return
    }
    public open prop propA: Int64 {
        get() {
            1
        }
    }
}

// 跨包同名类继承场景  Scenarios2
open class AClass {
    public func funcAold(): Unit {
        return
    }
    public prop propA: Int64 {
        get() {
            1
        }
    }
}
var bb = BClass().funcAold()
var bA = AClass().funcAold()

public open class BBClass {
    public func f12233(): Unit {
        return
    }
}

// 一级跨包拓展继承场景  Scenarios3
extend BBClass <: AInterface {
    public func funcAold(): Unit {
        return
    }
    public prop propA: Int64 {
        get() {
            1
        }
    }
}

var b = BBClass().funcAold()

enum ENUM <: AInterface {
    a | b | c

    public func funcAold() {}
    public prop propA: Int64 {
        get() {
            1
        }
    }
}
extend StringIterator {
    public func ffffnew() {}
    public prop aaaa11122: Int64 {
        get() {
            1
        }
    }
}

