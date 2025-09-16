// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

main() {
    var abstract : Int32 = 1
    var set : Int32 = 9
    var sealed : Int32 = 10
    sealed = 8
}

sealed class sealed {
    sealed() {}
}

class A {
    func sealed() {}
    func name() {
        var a = sealed()
    }
}

func name() {
    var a = sealed()
}


