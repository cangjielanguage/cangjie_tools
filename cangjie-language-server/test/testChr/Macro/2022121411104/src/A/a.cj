// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package A

from std import unittest.*
from std import unittest.testmacro.*

@Test class BCA{
    func gds(){}
}
@Test func ffss(){}
//@Test var yy =33

//@TestCase class BCA2{}

var ew1 = BCA().gds()
var ew2 = BCA().initCases()
//var ew3 = BCA().printResult()
