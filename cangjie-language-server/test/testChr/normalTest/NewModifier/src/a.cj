// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

//修饰符用例
import default.victor.*
open public class CURB <: A{
    init(defaultA:Int64){super(defaultA,defaultA)}
}

public class C <: CURB{
    init(defaultA:Int64){super(defaultA);}
    func cc(){
        // this.Afunc(1)
        super.Afunc(1,2)
        super.Afunc(1,2,3,3)
    }
    
}

// 构造函数 场景
let init_default = A(1)
let init_public = A(1,2) //包外只有public可以用ok
let init_private = A(1,2,3)
let init_protect = A(1,2,3,4)
// 一般成员函数 场景
let func_default = init_public.Afunc(1)
let func_public = init_public.Afunc(1,2) //包外一般函数只有public可以用ok
let func_private = init_public.Afunc(1,2,3)
let func_protect = init_public.Afunc(1,2,3,4)

// 本包的 继承类的场景
let curPkg_init_default = CURB(1)

let curPkg_default = curPkg_init_default.Afunc(1)
let curPkg_public = curPkg_init_default.Afunc(1,2) 
let curPkg_private = curPkg_init_default.Afunc(1,2,3)
let curPkg_protected = curPkg_init_default.Afunc(1,2,3,4)

// 本包的 深层继承类的场景
let curPkg_init_default_deep = C(1)

let curPkg_default_deep = curPkg_init_default_deep.Afunc(1)
let curPkg_public_deep = curPkg_init_default_deep.Afunc(1,2)
let curPkg_private_deep = curPkg_init_default_deep.Afunc(1,2,3)
let curPkg_protected_deep = curPkg_init_default_deep.Afunc(1,2,3,4)


//测试
let t1 = A. //public
let t2 = A(1,1).  //public
let t3 = B.   //public
let t4 = B(1).   //public
let t5 = C.   //public
let t6 = C(1).  // CC public

