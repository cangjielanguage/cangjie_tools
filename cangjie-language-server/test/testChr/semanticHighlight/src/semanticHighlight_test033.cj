// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class Base33 {
    protected var one:int32 = 1
    private var a: int32 = 0
    init(a: int32) {
        this.a = a
    }

    public func add(a: int32, b: int32):int32 {
        return a + b + this.one
    }

    open func add2(a: int32, b: int32):int32 {
        return a + b
    }
}
public class Data331 <: Base33 {
    var res = one
    static var res1:int32 = 1

    init(a: int32) {
        super(1)
        this.res = a + super.one
    }
    override func add2(a: int32, b: int32) {
        return a + b + super.add(1,2)
    }
    static func add3(a: int32, b: int32) {
        return a + b
    }
}

func semanticHighlight_test033(): int64 {
    var a:Data33 = Data33(1)
    var b:Base33 = Base33(1)
    var c = a.res
    var d = a.add2(1,2)
    var e = Data33.res1
    var f = Data33.add3(1,2)
    return 1
}

