// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos

import ohos.ark_interop.*
import ohos.ark_interop_macro.*

@Interop[ArkTS]
public func add(): Int8 {
1
}

@Interop[ArkTS]
public func addEntry(): Int8 {
1
}

@Interop[ArkTS]
public func getClassDemoInstance(): ClassDemo {
    ClassDemo(1.0)
}


@Interop[ArkTS]
public class ClassDemo {
    public var value2: Float64 = 1.0
    public let value3: Float64 = 1.0
    @Interop[ArkTS, Invisible]
    public var value4: Float64 = 1.0

    public init(a: Float64) {
        value2 = a
    }

    public func foo(a: Float64 ): Float64 {
        a
    }

    @Interop[ArkTS, Invisible]
    public func foo2(): Float64 {
        1.0
    }

    public mut prop id: Float64 {
        get() {
            value2
        }
        set(value) {
            this.value2 = value
        }
    }

    public prop id2: Float64 {
        get() {
            value3
        }
    }

    @Interop[ArkTS, Invisible]
    public prop id3: Float64 {
        get() {
            value4
        }
    }
}

@Interop[ArkTS]
public interface InterfaceDemo {
    mut prop id: Float64
    func foo(a!: Float64): Float64
}

@Interop[ArkTS]
public func doInterface(a: InterfaceDemo): Float64  {
    return a.foo(a: a.id)
}
