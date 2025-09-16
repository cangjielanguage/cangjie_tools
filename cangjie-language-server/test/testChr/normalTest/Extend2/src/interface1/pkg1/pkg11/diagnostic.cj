// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface1.pkg1.pkg11

import cangjieextend.interface1.pkg1.Foo

extend Foo {
    private func ppp() {
        this.foo() // ok
        this.fooProtected()    // ok
        this.fooInternal() // ok
    }
}
