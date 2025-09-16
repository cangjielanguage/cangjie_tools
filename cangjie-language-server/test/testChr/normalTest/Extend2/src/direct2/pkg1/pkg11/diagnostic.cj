// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.direct2.pkg1.pkg11

import cangjieextend.direct2.pkg1.I2
// import cangjieextend.direct2.pkg1.I3
import cangjieextend.direct2.pkg1.A2
import cangjieextend.direct2.pkg1.A3
import cangjieextend.direct2.pkg1.I3

let a1 = A3<Int32>().foo2() // ok
let a2 = A3<Int64>().foo3(1, 2) // ok
let a3 = A3<Int64>().foo2() // not ok, foo2 is not an extension of Int64
let a4 = A2<Int64>().foo33() // ok
let a5 = A2<Int64>().fooPrivate33() // not ok, the private method cannot be accessed
