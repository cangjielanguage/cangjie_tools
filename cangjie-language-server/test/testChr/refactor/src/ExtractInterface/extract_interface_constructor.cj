// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/16
 */

public class Person {
    public func printName(): String {
        return "person"
    }
    public func classOnly(): String {
        return "class only"
    }
}
class Consumer {
    var cached: Person = Person()
    func consume(person: Person): String {
        return person.printName()
    }
    func build(): Person {
        return Person()
    }
    func keepConcrete(person: Person): String {
        return person.classOnly()
    }
}
