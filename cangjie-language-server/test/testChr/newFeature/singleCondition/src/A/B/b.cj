// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.A.B
import default.C.*

@When[feature1 == "cat"]
func test1() {

}

@When[feature2 == "cat"]
func test2() {
    
}

@When[feature1 == "dog"]
func test3() {

}

@When[feature2 == "bird"]
func test4() {
    
}


func use() {
    test1()
    test2()
    test3()
    test4()
    test5()
}
