// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


var TC006_def_var = "test0001"
let globalvarTC006 = 123456


class TC006_fn01 {
    var TC006_def_var = 123456

    func funcA(){
        var Avar = TC006_def_var
        return Avar
    }

    func funcB(){

        var Bvar = TC006_def_var
        return Bvar
    }

    func funcC(){
        var Cvar = TC006_def_var
        return Cvar
    }
}

class TC006_fn02{
    func def_funcA(){
        var aaa = TC006_def_var
        return aaa
    }
    func def_funcB(bbb:String){
            if bbb == TC006_def_var{
                print(TC006_def_var)
            }
    }
    func def_funcC(ccc:String){
            TC006_def_var = ccc
            var ddd = TC006_def_var
            return ddd
            }
    func def_funcD(){
            var eee = def_funcC(TC006_def_var)
            print(eee)
            }
    }




