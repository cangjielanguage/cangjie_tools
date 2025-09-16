// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


public var sss1 = 1

class AAAA1 {
    prop aa: Int64 {
        get() {
            return sss
        }
    }
    var sss = 1
}

extend AAAA1 {
    func name() {

    }
}

struct AAAA2 {
    var sss = 1
}

extend AAAA2 {
    func name() {

    }
}

interface AAAA3 {
    func test() {}
}

extend AAAA1 <: AAAA3 {
    func name1() {

    }
}

enum AAAA4 {
    A | B | C
}

extend AAAA4 {
    func name() {

    }
}

main() {}

