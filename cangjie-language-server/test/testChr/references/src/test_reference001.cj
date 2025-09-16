// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

var atest : int32 = 1
var def_var = 123456
let globalVar = 333

public enum Timeunit {
    Year | Month | Day | Hour
}

public class Basec {
    var atest = 1
    protected var two = 2
    private var seven = 7
    static var three = 3
    protected static var five = 5
    private static var six = 6
    
    var par1 = atest
    init() {
        var par2 = atest
    }

    func test() {
        var par3 = atest
        var four = three
        return four
    }
}


class fn01 {
    var def_var = 123456

    func funcA(){
        var Avar = def_var
        return Avar
    }

    func funcB(){

        var Bvar = def_var
        return Bvar
    }

    func funcC(){
        var Cvar = def_var
        return Cvar
    }
}

class fn02{
    func def_func(){
        var aaa = "propValue"
        return aaa
    }

    func fn02_funcA(){
        var bbb = def_func()
        return bbb
    }
    func fn02_funcB(){
        var ccc = def_func()
        return ccc
    } 
    func fn02_funcC(){
        var ddd = def_func()
        return ddd
    }
  }
