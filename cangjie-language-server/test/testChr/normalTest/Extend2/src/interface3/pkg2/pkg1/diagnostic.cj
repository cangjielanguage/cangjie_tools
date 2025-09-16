// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface3.pkg2.pkg1

import cangjieextend.interface3.pkg2.I2
import cangjieextend.interface3.pkg2.I3
import cangjieextend.interface3.base.Foo

class D{}

// This test case shouldn't diag, extend can be exported nomally.
let a1 = Foo<D>().foo2()   // ok
let a2 = Foo<D>().foo3()   // ok
