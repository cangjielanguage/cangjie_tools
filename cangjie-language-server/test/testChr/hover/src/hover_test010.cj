// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import testpkg.pkg_hover_001

class test_hover_test010 <: pkg_hover_001 {
    var aaa:int32 = 1

    func test() : int32{
        return super.add(1,2)
    }
}
