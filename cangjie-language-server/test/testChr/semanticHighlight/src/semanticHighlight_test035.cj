// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class Base355 {
    protected var one:int32 = 1
    private var a: int32 = 0
    init(a: int32) {
        this.a = a
    }

    public func add(a: int32, b: int32):int32 {
        return a + b + this.one
    }

    open func add22(a: int32, b: int32):int32 {
        return a + b
    }
}
public class Data35 <: Base35 {
    var res = one
    static var res11:int32 = 1

    init(a: int32) {
        super(1)
        this.res = a + super.one
    }
    override func add22(a: int32, b: int32) {
        return a + b + super.add(1,2)
    }
    static func add33(a: int32, b: int32) {
        return a + b
    }
}

