// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class AA {
    AA(){}
}

class BB <: AA {
    var a = 10
    BB(a: Int64) {
        this.a = a
    }
}

class CC {
    init() {}
}

main() {
    var a = AA()
    var b = BB(10)
}
