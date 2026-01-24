// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

features {user.lib.arch.big}
package lib 

public specific func message(): Unit {
    println("Some example message")
}

public specific func message1(): Unit {
    println("Some example message")
}

public specific func name333(): Unit {
    aaa()
    AAA
}

func name1() {
    message()
    aaa()
    name333()
    AAA().aa
}

class AAA {
    public let aa = 1
}