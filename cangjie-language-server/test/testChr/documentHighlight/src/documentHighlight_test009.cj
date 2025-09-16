// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class A09 {
    public var publicAvar:int32 = 1
    protected var protectedAvar:int32 = 1
    private var privateAvar:int32 = 1
    static var staticAvar:int32 = 1

    init(a: int32) {
        this.publicAvar = a
        
    }

    public func publicAfunc(a: int32){
        return a + this.publicAvar
    }

    protected func protectedAfunc(a: int32, b: int32){
        return a + b + this.publicAvar
    }

    private func privateAfunc(a: int32, b: int32){
        return a + b + this.privateAvar
    }
    static func staticAfunc(a: int32, b: int32){
        return a + b
    }

    open func publicAfunc(a: int32, b: int32) {
        return a + b
    }

    func publicAadd(a: int32) {
        return a + 100
    }
}

class B09 <: A09 {
    public var publicBvar:int32 = 1
    protected var protectedBvar:int32 = 1
    private var privateBvar:int32 = 1
    static var staticBvar:int32 = 1

    public func publicBfunc(a: int32){
        return a + this.publicAvar
    }

    protected func protectedBfunc(a: int32, b: int32){
        return a + b + this.publicAvar
    }

    private func privateBfunc(a: int32, b: int32){
        return a + b + this.protectedBvar
    }
    static func staticBfunc(a: int32, b: int32){
        return a + b
    }

    open func publicBfunc(a: int32, b: int32) {
        return a + b + this.privateBvar
    }

    func publicBadd(a: int32) {
        return a + 100
    }
    init(a: int32) {
        this.publicBvar = super.protectedAvar
    }

    override func publicAfunc(a: int32, b: int32){
        return a + b + super.publicAadd(1)
    }
}

func test_09(){
    var valueB:B09 = B09(1)
    var valueA:A09 = A09(1)
    var a = valueA.publicAvar
    var a1 = A09.staticAvar
    var b = valueB.publicAvar
    var b1 = valueB.publicBvar
    var b2 = B09.staticAvar
    var b3 = B09.staticBvar
 }
