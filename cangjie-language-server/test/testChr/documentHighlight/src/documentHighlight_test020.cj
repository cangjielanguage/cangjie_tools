// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I20 {
    let aclass: int32 = 1
    func test_I20(a:int32):int32
}

class Base20 <: I20 {
    static var classa = 1
    static var Class = 2
	var a:int32 = 3
    init() {
        this.a = I20.aclass
    }
    init(a: int32) {
        this.a = a
    }

    func test_I20(a:int32):int32 {
        return a
    }
}

func test_Base20(){
    var value:Base20 =  Base20()
    var b = value.test_I20(100)
}

