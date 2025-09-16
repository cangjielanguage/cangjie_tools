// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I21 {
    let aclass: int32 = 1
    default func getNum():int32 {
        return 1
    }
    
    static func getSum():int32 {
        return getNum()
    }
}

class Base21 <: I21 {
    static var classa = 1
    static var Class = 2
	var a:int32 = 3
    init() {
        this.a = I21.aclass
    }
    init(a: int32) {
        this.a = a
    }
    static func add():int32 {
        return I21.getSum()
    }

    func add(a:int32):int32 {
        return getNum()
    }
}

