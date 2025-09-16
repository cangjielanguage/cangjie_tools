// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package B 

import A.*

@M1
func color() {
    var aa = 10
}

@M2
class A1 {
    func f1() {

    }

    var a = 10
}

@M3
class A1 {

}

func test() {
    var a1 = A1()
    a1.f1()
    a1.hehe()
}
