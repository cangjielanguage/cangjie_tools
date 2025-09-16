// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package overrideCompletion.pkg1

public interface I1<T> {
    func fabni1(): Unit

    func fabni2(a: T): T

    func fabni3(a: T) {
        return a
    }
}

open public class C1<T> {
    open public func fabnc1() {

    }

    open public func fabnc2(a: Int64) {
        return a
    }

    open public func fabnc3(a: Int64) {

    }

    open public func fabnc4(a: T): T {
        return a
    }

    open public func fabnc5(a: T): Unit {

    }    
}

open public class C2<K> <: C1<K> {
    open public func fabnc4(a: K): K {
        return a
    }

    open public override func fabnc5(a: K): Unit {

    }

    open public func fabnc6(a: K): Unit {

    }
}