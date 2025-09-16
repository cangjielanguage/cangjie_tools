// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import pkg.B
import pkg.*
class A {
    A(){}
}

class AA {
    var a: Int32
    init(a1:Int32){
        a = a1
    }
}

class AI <: BI {

}

class AI1 <: pkg.BI {

}

func main(){
    var ab = AA(10)
    var c : pkg.B
    var ci : pkg.BI
    ab.a = 20
    return 0
}

