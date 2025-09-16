// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface I39 {
    let aaa: int32 = 1

    default func getNum1():int32 {
        return 1
    }
    
    static func getSum1():int32 {
        return 1
    }
}

class Base39 <: I39 {
	var a:int32 = 3
    init(a: int32) {
        this.a = a
    }

    func add(a: int32, b: int32):int32 {
        return a + b + I39.getSum()
    }

    func add(a: int32):int32 {
        return a + getNum()
    }

    func add():int32 {
        return I39.aa
    }
}
