// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func test6(): Int64 {
    var ff = Coo<AAoo, Rune>().a
    return 0
}

class Coo<T, V> where T <: Boo {
    public prop a: String {
        get() {
            return T.foo<V>()
        }
    }
}

open class Boo {
    public static func foo<V>() {
        return "abc"
    }
}

class AAoo <: Boo {}
