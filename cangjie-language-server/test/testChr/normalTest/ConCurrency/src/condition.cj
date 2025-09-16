// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.sync.*

var m1 : MultiConditionMonitor = MultiConditionMonitor()
var cR : ConditionID = m1.newCondition()
var cR1 : Conditio

func test4() {

    spawn {
        m1.lock()
        while (flag) {
            m1.wait(cR)
        }
        count = count + 1
        m1.unlock()
    }
    m1.lock()
    flag = false
    m1.unlock()
    m1.notify(cR)
}
