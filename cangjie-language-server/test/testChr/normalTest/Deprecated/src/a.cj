// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package vscode_cangjie

@Deprecated
func testcc(s: String, ss: String, @Deprecated b!: Int64 = 1) {
    var c = b
}

@Deprecated
type AssociatedTypeT = Int64

@Deprecated class C {
    @Deprecated
    let a = 1

    @Deprecated
    func f(
        @Deprecated param!: Int64 = 1
    ) {}

    @Deprecated
    C() {}

    @Deprecated
    init(b: Int64) {}

    @Deprecated
    static func st_f() {}

    @Deprecated
    static let y = 1
}



@Deprecated
class Clazz {
    @Deprecated
    public var z: Int64 = 10
}

@Deprecated
enum RGBColor {
    | Red | Green | Blue
}

@Deprecated
struct testStruct {
    @Deprecated
    public var s: AssociatedTypeT

    @Deprecated
    public init(s: AssociatedTypeT) {
    	this.s = s
    }

    @Deprecated
    operator func ==(other: C): Bool {
      return true
    }
}
