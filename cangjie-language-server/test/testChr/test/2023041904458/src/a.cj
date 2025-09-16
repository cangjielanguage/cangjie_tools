// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

//2
import default.AA.*

// case 1
class BO <: AO {
    public mut override prop i1: Int64 {
        get() {
            1
        } // ok
    }
    mut static redef prop i22222: Int64 {
        get() {
            1
        } // ok
    }
}

// case 2
class BO2 <: AO {
    public mut override prop i1: Int64 {
        set(v) {} // ok
    }
    mut static redef prop i22222: Int64 {
        set(v) {} // ok
    }
}

func test12() {
    var a = AO().i1
    var b = BO().i1
    var c = BO2.i22222
}


