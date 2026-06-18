// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/17
 */
package ohos_app_cangjie_entry.IntroduceParameter

func introduceParameterSample(base: Int64) {
    var value = base + 1
}

func callIntroduceParameterSample() {
    introduceParameterSample(10)
}

func localReferenceParameter(base: Int64): Int64 {
    let delta = 2
    return base + delta
}

func twoOriginalParameters(left: Int64, right: Int64): Int64 {
    return left + right
}

func literalParameterSample(): Float64 {
    return 0.9 + 12.1
}

func localOnlyExpression(): Int64 {
    let left = 1
    let right = 2
    return left + right
}

func mixedParams(a: Int64, b!: Int64, c!: Int64): Int64 {
    a + b + c
}

public func testMixedParams(): Unit {
    let r1 = mixedParams(1, b: 10, c: 100)
    let r2 = mixedParams(2, c: 200, b: 20)

    println("Mixed param test: ${r1}, ${r2}")
}


open class Animal2 {
    var name: String

    init(name: String) {
        this.name = name
    }

    func speak() {
        println("${this.name} speaks")
    }

    public open func getInfo(): String {
        return "Animal: ${this.name}"
    }
}

class Dog <: Animal2 {
    init(name: String) {
        super(name)
    }

    public override func getInfo(): String {
        return "Dog: ${this.name}"
    }
}

func main1() {
    let dog = Dog("??")
    dog.speak()
    println(dog.getInfo())
}


func greet(name!: String = "World", greeting!: String = "Hello"): String {
    "${greeting}, ${name}!"
    "test" + name
}

public func testDefaultValue(): Unit {
    let r1 = greet()
    let r2 = greet(name: "Cangjie")
}

func compoundAssignSample(a: Int64): Unit {
    a += 2
    a -= 2
    a <<= 2
    a ^= 2
}

func callCompoundAssignSample(): Unit {
    var value: Int64 = 10
    compoundAssignSample(value)
}

func bitCompoundAssignSample(): Unit {
    var bit: Int32 = 0b0001
    bit >>= 1
    bit |= 0b1000
    bit &= 0b1111
    bit ^= 0b1111
}

func outer(a: Int64): Int64 {
    func inner(b: Int64): Int64 {
        b * 2
    }
    inner(a) + 1
}

public func testNestedFunction(): Unit {
    let result = outer(10)
    println("outer(10) = ${result}")
}
