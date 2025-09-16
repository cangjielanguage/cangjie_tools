// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test.C
import A.B.C.D.E.F.G.H.I.J.K.BasecTest
class Data1 <: Basec & Basei {
    var resc = atest
    var resi = test()
    static func staFun() {
        var par = test1()
    }
    var test3 : structer1 = structer1(4)
    var time1 = Timeunit.Year
}

var value6 : Basec = Basec()
var value7 = value6.test()


func test6(): int64 {
    var value1 : Data1 = Data1()
    if ( value1.resc != 1 || value1.resi != 1 ) {
        return 1
    }
    return 0
}

open class Data2 {
    var testData : BasecTest = BasecTest()
}
