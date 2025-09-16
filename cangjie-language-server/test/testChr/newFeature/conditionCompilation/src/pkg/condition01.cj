// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkg

@When[cc11 == "c1"]
func testC1() {

}

@When[cc22 == "c2"]
func testC2() {

}

@When[cc33 == "c3"]
func testC3() {

}

@When[cc55 == "c5"]
func testC5() {

}




func test() {
    testC1()

    testC2()

    testC6()
}
