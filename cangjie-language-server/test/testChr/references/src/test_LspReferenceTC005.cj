// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


var TC005_def_var = "test0001"
let globalvar = 123456


class TC005_fn01 {
    var TC005_def_var = 123456

    func funcA(){
        var Avar = TC005_def_var
        return Avar
    }

    func funcB(){

        var Bvar = TC005_def_var
        return Bvar
    }

    func funcC(){
        var Cvar = TC005_def_var
        return Cvar
    }
}

class TC005_fn02{
    func def_func(){
        var aaa = TC005_def_var
        return aaa
    }
  }




