// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/16
 */

package ohos_app_cangjie_entry.ExtractInterface

interface Printable {
    func printName(): String
}

interface Locatable {
    func location(): String
}

interface Named <: Printable {
    public func displayName(): String {
        return printName()
    }
}

enum Color {
    Red | Blue

    public func rgb(): Int64 {
        return 0
    }

    static func defaultColor(): Color {
        return Red
    }
}

open class Person <: Printable & Locatable {
    var name: String

    init(name: String) {
        this.name = name
    }

    public func printName(): String {
        return name
    }

    protected func age(): Int64 {
        return 18
    }

    static func buildDefault(): Person {
        return Person("default")
    }

    private func secret(): String {
        return "hidden"
    }

    public func location(): String {
        return "earth"
    }
}

class EmptyBox {
    var value = 0
}

struct Point {
    public func x(): Int64 {
        return 1
    }

    func y(): Int64 {
        return 2
    }
}

extend String {
    public func firstChar(): String {
        return this[0].toString()
    }
}

class GenericBox<T> where T <: ToString {
    var value: T

    init(value: T) {
        this.value = value
    }

    public func get(): T {
        return value
    }

    func stringify(): String {
        return value.toString()
    }
}

class Consumer {
    func consume(person: Person): String {
        return person.printName()
    }

    func locate(person: Person): String {
        return person.location()
    }
}

open class BaseGreeter {
    public func greet(): String {
        "Hello"
    }
}

extend BaseGreeter {
    public func greetLoudly(): String {
        "${greet()}!!!"
    }
}

interface NamedType {
    static func typeName(): String
}

class ClassA <: NamedType {
    public static func typeName(): String {
        "ClassA"
    }
}

class ClassB <: NamedType {
    public static func typeName(): String {
        "ClassB"
    }
}

public struct Counter {
    var value: Int64 = 0
    public mut func inc(): Unit {
        value += 1
    }
}
