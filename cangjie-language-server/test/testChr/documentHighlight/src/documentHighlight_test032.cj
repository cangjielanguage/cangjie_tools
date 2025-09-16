// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import testpkg.iPkg

class testIpkg <: iPkg{
    var aa:int32 = 0
    init() {
        aa = iPkg.aclass
    }

    func foo():int32{
        return 1
    }
}


func pkg_test32(): int32 {
    var a = iPkg.getSum()
    return 1
}
