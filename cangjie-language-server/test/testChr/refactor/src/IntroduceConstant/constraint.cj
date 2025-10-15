// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.IntroduceConstant

func test1() {
    // check1 apply1
    var const1 = 1 * 1 + 1 * 1
    // check2
    var const2 = const1 + 1
    // check3
    const2++
}

//check4
const const3 = "xxxx"

//check5
const const4 = const3 + "x"

class AA {
    //check6
    static const a1 = 10.0

    func func1() {
        // check7
        var aa = a1 * 1.0
        // check8
        var bb = 100
    }

    func func2() {
    }
}

const const5 = 10

var bb = 10

const func func2(a1: Int64, a2: Int64) {
    return 1
}

const func func3(aa: Int64) {
    return 1
}

func func1() {
    // check9 check10 check11
    var a1 = 10 + const5 + func2(1 + const5 + bb, 1 + func3(const5)) // test
}


extend String {
	func test1() {
        // check12
        var aa = "xxx"
	}
}