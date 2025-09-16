// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import test.Basec111
import test.Timeunit111
import test.structer222
import test.Basei222
open class Basec1 <: Basei222{
    var one : int32 = 1
}

func testFunc() {
    var test1  = Basec111.test1()
    var value2 : Basec111 = Basec111()
    var test2  = value2.test()
    var time1 = Timeunit111.Year
    var test3 : structer222 = structer222(4)
    var i = test3.a
}

