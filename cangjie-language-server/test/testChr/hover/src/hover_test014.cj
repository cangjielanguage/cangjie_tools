// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import testpkg.iPkg_hover

class test_hover_test014 <: iPkg_hover {
    var aaa:int32 = 1

    func foo():int32 {
        return 1
    }
}

func run_test_hover_test014(): unit {
    var value :test_hover_test014 = test_hover_test014()
    var a = value.foo()
}
