// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

let def_var02:Int32 = 100

class Test002 {
	var a:Int32 = 3
    init(a: Int32) {
        this.a = a
    }

    func add(a: Int32, b: Int32):Int32 {
        return a + b + def_var02
    }
}

class Test0022 {
	var a:Int32 = 3
    init(a: Int32) {
        this.a = a
    }

    func add(a: Int32, b: Int32):Int32 {
        var def_var02_local:Int32 = 100
        return a + b + def_var02_local
    }
}
