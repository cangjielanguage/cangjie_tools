// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

extend String {
    func printSize() {
        print("")
    }
}

class Foo1 {}
extend Foo1 {
    func f() {}
}

class Foo<T> {
    var item: T
    init(it: T) {
        item = it
    }
}
interface Eq<T> {
    func equals(other: T): Bool
}
extend<T> Foo<T> where T <: Eq<T> {
    func equals(other: Foo<T>) {
        item.equals(other.item)
        0
    }
}

class Foo2 {}
extend Foo2 {
    private func f1() {}
    public func f2() {}
    func f4() {}
}

struct Foo3 {
    var i = 0
}
extend Foo3 {
    mut func f() { // ok
        i += 1
    }
}

extend Int64 {
    prop i: Int64 {
        get() {
            0
        }
    }
}

