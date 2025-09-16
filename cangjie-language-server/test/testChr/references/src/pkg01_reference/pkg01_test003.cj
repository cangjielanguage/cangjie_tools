// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg01_reference
class test003_fn01{
    var def_var03 = "test003"
    public func def_func_test003(){
        if def_var03 != test001_fn01.def_func_test001(){
            var ccc = test001_fn01.def_func_test001()
            print(ccc) 
        }
        return 1
    }
}
