// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class Test003 {
	var a:Int32 = 3
    init(a: Int32) {
        this.a = a
    }

    func def_func001(a: Int32, b: Int32):Int32 {
        return a + b
    }
}

class Test0032 {
	var a:Int32 = 3
    init(a: Int32) {
        this.a = a
    }

    func def_func002(a: Int32, b: Int32):Int32 {
        var value:Test003 = Test003(1)
        return a + b + value.def_func001(1,2)
    }
}
