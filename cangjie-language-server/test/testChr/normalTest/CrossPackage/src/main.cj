// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import default.A.*

// struct跨包继承场景  Scenarios7
struct MaimStruct <: AInterface {
    public func funcAold() {}
}

let c = MaimStruct().funcAold()

interface MaimInterface <: AInterface {}

// 跨接口继承场景  Scenarios8
class MCLASS <: MaimInterface {
    public func funcAold() {}
}

let cc = MCLASS().funcAold()

interface Eq<T> {
    func eq(other: T): Bool
}

interface Ord<T> where T <: Eq<T> {
    func eq1(other: T): Bool
}

class G <: Eq<Int64> {
    public func eq(other: Int64): Bool {
        return true
    }
    public mut prop aaaa111: Int64 {
        get() {
            1
        }
        set(a) {
            a + 1
        }
    }
}

func foo<T>(a: T) where T <: Ord<T> {
    a.eq(a)
    a.eq1(a)
}

let csa: String = ""

class dddaa <: ArithmeticException {
    dddaa() {
        super()
    }
    public override func getClassName(): String {
        return "ArithmeticException"
    }
}

class dddaabb <: ArithmeticException {
    dddaabb() {
        super()
    }
    public override func getClassName(): String {
        return "ArithmeticException"
    }
}

