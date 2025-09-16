// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package Any 
//所有非interface默认实现Any接口

class AA{
    public var a1 =0
}

func testA2() {
    var ii:Any= AA()
    var ii1= AA()
    ii1.a1
    
}


