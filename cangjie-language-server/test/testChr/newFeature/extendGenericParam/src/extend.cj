// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


class Foo1<TTT1,TTT22> {
  Foo1(a: Int64){}
}

extend<TTT1,TTT22> Foo1<TTT1,TTT22>{
   func name(a: TTT1)  {

       var aa: TTT22
   }
}

extend<TTT1,TTT22> Foo1<TTT1,TTT22>{
   func name2(a: TTT1)  {

       var aa: TTT22
   }
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
    func equals(other: Foo<T>): Bool {
        if (this.item.equals(other.item)) {
            return true
        }
    return false
    }
}
