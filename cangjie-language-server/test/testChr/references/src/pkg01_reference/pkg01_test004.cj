// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package pkg01_reference
class test004_fn01{
    var def_var04 = "test004"
    public func def_func_test004(){
        if def_var04 !=test001_fn01.def_func_test001(){
            var ddd = "test004++"
            if ddd != test001_fn01.def_func_test001(){
                print(test001_fn01.def_func_test001())
            }
            print(ddd)
        }
    }
}
