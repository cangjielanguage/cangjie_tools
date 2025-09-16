// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package tesrt4.BBB

import tesrt4.AAA.*

@AA3[func fa4(){}] func fa5<T>(a:Int64,b:T){}
var b5 = fa5<Bool>(1,true)

@AA3[struct Sa4{var bbx = 9 Sa4(){}}]
struct Sa5{
    var ab =8
    Sa5(x:Int64){
        this.ab = x
    }
    static func sf5(){}
    func sf6(x:Bool){}
}

func test4(){
    // var d1 = Sa1().num1
    // var d2 = Sa2()
    var d3 = Sa4().bbx
    var d4 = Sa5(34).sf6(true)
    var d5 = Sa5.sf5()
    var d6 = Sa5(4).ab
}

