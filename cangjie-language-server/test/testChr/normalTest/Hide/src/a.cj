// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos

import ohos.labels.Hide

@Hide
open class AAA {
    open public func xxxx() {

    }
}

open class BBB {
    @Hide
    open public func yyyy() {

    }

    @Hide
    open public prop name: String {
        get() {
            return "BBB"
        }
    }

    @Hide
    public let value: Int64 = 100
}

@Hide
extend BBB {
    // @Hide
    public func zzzz() {

    }
}

extend BBB {
    @Hide
    public func pppp() {

    }
}

open class CCC {
    // @Hide
    public CCC (
        @Hide
        public let x: Int64,
        public let y: Int64
    ){

    }
}

open class DDD {
    @Hide
    public init() {

    }
}

enum EEE {
    A |
    B |
    @Hide
    AA

    @Hide
    public func isHidden(): Bool {
        return true
    }
}

@Hide
public var hiddenGlobal: Int64 = 42

@Hide
public func hiddenFunc(): Int64 {
    return 123
}

public interface T2<T> {
    @Hide
    func bar() {
    }
}