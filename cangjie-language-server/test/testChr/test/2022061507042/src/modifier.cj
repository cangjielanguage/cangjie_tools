// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public import pkg_modi.*

open public class Modi <: ModifierA{
    init(deMoA:Int64){
        super(deMoA,deMoA)
    }


    //测试
    let init_1 = ModifierA(1, 1) //public
    let func_1 = init_1. //public 
    let s_func_1 = modifierA //public  protect
}

public class ModifierC <: Modi{
    init(deMoA:Int64){
        super(deMoA)
    }

    func moc(){
        super.MofuncD(1, 1, 1, 1) //protect public
        super.MofuncD(1,2,3,4)
    }
}

// 构造函数
let init_default = ModifierA
let init_public = ModifierA(1,-2)
let init_private = ModifierA(1,-2,3)
let init_protect = ModifierA(1,-2,3,-4)

// 一般成员函数 场景
let func_default = init_public.MofuncA(1)
let func_public = init_public.MofuncB(1,-2)
let func_private = init_public.MofuncC(1,-2,3)
let func_protect = init_public.MofuncD(1,-2,3,-4)

// 本包的 继承类的场景
let inherPkg_init_default = Modi(2)

let inherPkg_default = inherPkg_init_default.MofuncA(1)
let inherPkg_public = inherPkg_init_default.MofuncB(1,-2)
let inherPkg_private = inherPkg_init_default.MofuncC(1,-2,3)
let inherPkg_protected = inherPkg_init_default.MofuncD(1,-2,3,-4)


// 本包的 深层继承类的场景
let inherPkg_init_default_deep = ModifierC(3)

let inherPkg_default_deep = ModifierC(3).MofuncA(1)
let inherPkg_public_deep = ModifierC(3).MofuncB(1,-2)
let inherPkg_private_deep = ModifierC(3).MofuncC(1,-2,3)
let inherPkg_protected_deep = ModifierC(3).MofuncD(1,-2,3,-4)


//测试

let t1 = ModifierA.//public
let t2 = ModifierA(1,1).  //public 
let t3 = ModifierB.MofuncD(1, 1, 1, 1)   //public
let t4 = ModifierB(1).   //nothing because init(int64) is not public
let t5 = ModifierC   //public 
let t6 = ModifierC(1).Afunc(1, 1, 1, 1)  // CC public

let dd = "ssasa"
let ss = 

