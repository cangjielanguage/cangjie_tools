// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cangjieextend.direct3

import cangjieextend.direct3.pkg1.A

// When the definition of the direct extension and the extended type is in a different `package`, the extension is never exported and can only be used in the current `package`.
let a1 = A().a() // not ok
