// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I41 {
    let aa: int32 = 1
    func add(a: int32):int32
    default func getNum(a: int32):int32 {
        return 1
    }
    
    static func getSum(a: int32):int32 {
        return 1
    }
}

class Base41 <: I41 {
	var a:int32 = 3
    init(a: int32) {
        this.a = a
    }

    func add(a: int32, b: int32):int32 {
        return a + b + I41.getSum(1.0)
    }

    func add(a: int32):int32 {
        return a + getNum(1.0)
    }

    func add():int32 {
        return I41.aa
    }
}
