// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface1.pkg2

import cangjieextend.interface1.pkg1.Foo

public func comp() {
    Foo().foo() // ok
    Foo().fooProtected() // ok
    Foo().
}
