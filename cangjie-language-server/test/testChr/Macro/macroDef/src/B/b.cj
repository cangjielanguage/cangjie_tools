// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package B 

import A.*

@M2
class B1 {

}

func test() {
    var b = B1()
    b.hehe()
}

@M3
func test() {
    var a123 = 20
    a123 = 10
}
