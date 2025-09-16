// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class AAAAA {
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

    open public func publicAAAfunc(){
        return 100
    }
}
class BBBBB <: AAAAA {
    public var publicBvar:int32 = 1
    init() {
        super(1)
        this.publicBvar = super.publicAvar   
    }

    public func publicBBBfunc(){
        return super.publicAAAfunc(1,2)
    }

    override public func publicAAAfunc(){
        return 100
    }
}

func test_BBBBB():int32 {
    var value:BBBBB = BBBBB()
    var a = value.publicAAAfunc()
    var b = value.publicAAAfunc(1)
    var d = value.publicAvar
    var e = value.publicBvar
    return 1
}
