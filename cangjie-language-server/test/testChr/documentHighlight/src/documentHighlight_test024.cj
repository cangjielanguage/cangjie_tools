// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I24 {
    let aclass: int32 = 1
    default func getNum():int32 {
        return 1
    }
    
    static func getSum():int32 {
        return getNum()
    }
}

interface I2401 {
    let aa: int32 = 1
}

interface I24_Son <: I24 & I2401 {
    override default func getNum():int32 {
        return 2
    }
}

class Base24 <: I24_Son {
    static var classa = 1
    static var Class = 2
    var a:int32 = 3
    init() {
        this.a = I24_Son.aclass + I24_Son.aa
    }
    init(a: int32) {
        this.a = a
    }
}
