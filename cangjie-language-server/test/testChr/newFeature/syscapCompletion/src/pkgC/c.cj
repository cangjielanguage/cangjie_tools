// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package default.pkgC

import default.level.*

open class A {
    @!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
    public func f12a(): Unit {

    }

    @!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
    public func f12b(): Unit {

    }

    @!APILevel[11]
    public func f12c(): Unit {

    }

    @!APILevel[9, syscap: "SystemCapability.AI.InsightIntent"]
    public func f12d(): Unit {

    }

    public func f12f(): Unit {

    }

    @!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
    public var f12vara = 10

    @!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
    public var f12varb = 10

    public var f12varc = 10    

    private var a = 0

    @!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
    public mut prop f12propa: Int64 {
        get() {
            println("get")
            a
        }
        set(value) {
            println("set")
            a = value
        }
    }

    @!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
    public mut prop f12propb: Int64 {
        get() {
            println("get")
            a
        }
        set(value) {
            println("set")
            a = value
        }
    }

    public mut prop f12propc: Int64 {
        get() {
            println("get")
            a
        }
        set(value) {
            println("set")
            a = value
        }
    }      

    public func use() {
        f12
    }
}

func fb1() {
    let a = A()
    a.
}

class B <: A {
    @!APILevel[9, syscap: "SystemCapability.ArkUI.xxx"]
    public func f123a(): Unit {

    }

    @!APILevel[9, syscap: "SystemCapability.ArkUI.UiAppearance"]
    public func f123b(): Unit {

    }
    
    public func use() {
        f12
    }    
}