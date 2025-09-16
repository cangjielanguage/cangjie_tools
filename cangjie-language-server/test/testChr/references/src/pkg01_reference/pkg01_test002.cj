// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg01_reference
class test002_fn01{
    var def_var02 = "test002"
    public func def_func_test002(){
        var bbb = test001_fn01.def_func_test001()
        return bbb
    }
}

