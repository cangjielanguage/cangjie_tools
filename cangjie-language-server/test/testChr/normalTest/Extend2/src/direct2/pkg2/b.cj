// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.direct2.pkg2

/* import cangjieextend.pkg1.A

let a = A<Int64>().foo(1, 2) // I3 not imported, cannot use `foo`. */

import cangjieextend.direct2.pkg1.A3
import cangjieextend.direct2.pkg1.I3

let a2 = A3<Int64>().foo2() // not ok
let a3 = A3<Int64>().foo3(1, 2) // ok


