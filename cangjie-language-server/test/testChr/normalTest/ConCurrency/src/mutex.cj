// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.sync.*

var m = MultiConditionMonitor()
var c = m.newCondition()
var flag : Bool = true
var count : Int64 = 0

func test2() : Int64 {
    spawn {
        m.lock()
        while (flag) {
            m.wait(c)
        }
        count = count + 1
        m.unlock()
    }
    m.lock()
    flag = false
    m.unlock()
    m.notify(c)
    return 0
}

var m2 = Mult



