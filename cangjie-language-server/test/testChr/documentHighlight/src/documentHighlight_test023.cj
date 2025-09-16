// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I23 {
    let aclass: int32 = 1
    default func getNum():int32 {
        return 1
    }
    
    static func getSum():int32 {
        return getNum()
    }
}

interface I23_Son <: I23 {
    override default func getNum():int32 {
        return 2
    }
}

class Base23 <: I23_Son {
    static var classa = 1
    static var Class = 2
    var a:int32 = 3
    init() {
        this.a = I23_Son.aclass
    }
    init(a: int32) {
        this.a = a
    }
}

func test_Base23():int32 {
    var value:Base23 = Base23()
    var a = value.getNum()
    return a
}
