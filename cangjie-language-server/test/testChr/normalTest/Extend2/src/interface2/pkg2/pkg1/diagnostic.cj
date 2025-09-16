// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface2.pkg2.pkg1

import cangjieextend.interface2.pkg2.I2
import cangjieextend.interface2.pkg2.I3
import cangjieextend.interface2.pkg2.I4
import cangjieextend.interface2.pkg2.F2
import cangjieextend.interface2.base.Foo

class D <: F2{}

let a2 = Foo<D>().foo2()   // ok
let a3 = Foo<D>().foo3()   // ok
let a4 = Foo<D>().foo4()   // ok
