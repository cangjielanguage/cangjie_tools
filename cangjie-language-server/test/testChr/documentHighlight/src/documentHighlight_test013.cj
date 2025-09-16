// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class AAA {
    public var publicAvar:int32 = 1
    init(a: int32) {
        this.publicAvar = a   
    }

    init() {
        this.publicAvar = 100
    }

    init(a: int32, b: int32) {
        this.publicAvar = a + b
    }

    public func publicAAAfunc(a: int32){
        return a
    }

    public func publicAAAfunc(a: int32, b: int32){
        return a + b
    }

    public func publicAAAfunc(){
        return 100
    }
}

func test_AAA(){
    var valueA:AAA = AAA(1)
    var valueB:AAA = AAA()
    var valueC:AAA = AAA(1,2)
 }
