// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

/**
 * Created on 2025/7/17
 */
package ohos_app_cangjie_entry.IntroduceField

class IntroduceFieldSample {
    func calc(base: Int64) {
        var value = 3.14 * base.toFloat64()
    }

    static func staticCalc(): Int64 {
        return 7
    }

    func calcExpression(): Float64 {
        return 0.9 + 12.1
    }

    func calcLocalDependency(): Float64 {
        var t = 0.1
        return t + 0.2
    }
}

func extractGlobalFieldSample() {
    var value = 42
}

interface IntroduceFieldInterfaceSample {
    func value(): Int64 {
        return 8
    }
}

class FieldInitializerRefs {
    var base: Float64 = 2.0

    func useMember(): Float64 {
        return this.base + 1.0
    }

    func useParameter(radius: Float64): Float64 {
        return radius + 1.0
    }

    func useLocal(): Float64 {
        let local = 1.0
        return local + 2.0
    }
}

class InsertBeforeMethodCases {
    var first: Int64 = 1

    func beforeLaterField(): Int64 {
        return 1 * 2
    }

    var later: Int64 = 3
}

class CommentedMethodFieldCases {
    // Keep this comment on the method.
    func commentedMethod(): Int64 {
        return 1 * 2
    }
}

// Keep this comment on the top-level function.
func topLevelWithComment(): Int64 {
    return 1 * 2
}

struct Temperature {
    func outer(a: Int64): Int64 {
        a+1
    }
}

public func testStringType(): Unit {
    let s1: String = "Hello Cangjie"
    let s2 = "World"

    // Interpolation string
    let name = "Alice"
    let age = 30
    let greeting = "Hello, ${name}! You are ${age} years old."

    // Multiline raw string
    let raw = #"""
        Line 1
        Line 2
        Line 3
        """#

    // Convert string to Rune array
    let runes = "nihao".toRuneArray()
}
