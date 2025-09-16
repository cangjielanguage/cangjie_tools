// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface1.pkg1

interface I { // not visible in package b
    func foo(): Unit
}

public class Foo {}
extend Foo <: I {
    public func foo(): Unit {}
    protected func fooProtected(): Unit {}
    internal func fooInternal(): Unit {}
    private func fooPrivate(): Unit {}
}


