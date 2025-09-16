// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.interface3.pkg1.pkg1

import cangjieextend.interface3.pkg1.I2
import cangjieextend.interface3.pkg1.I3
import cangjieextend.interface3.base.Foo

private class D1{}

let a2 = Foo<D1>().foo2()   // ok
let a3 = Foo<D1>().foo3()   // ok
