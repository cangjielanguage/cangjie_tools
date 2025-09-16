// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package MacroNodeCompletion
import MacroNodeCompletion.pkg.*

@Component
public class A {
    @State
    public var a = 1

    public func foo() {
        a
    }
}


main(): Int64 {
    println("hello world")
    return 0
}
