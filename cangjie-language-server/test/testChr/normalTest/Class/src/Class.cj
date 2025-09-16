// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


class  A{
	 public var A_Public_var = 10
	 private var A_Private_var = 11
	 protected var A_Protected_var = 12
     init( )  {
	     A_Private_var =5
     }
     init(a:Int64 )  {
         A_Private_var =a
     }
	 open func A_open_func( ) {
	    return 1
     }
}
class C <: D{}

class B <: A{
     public var B_Public_var = 10
     private var B_Private_var = 11
     protected var B_Protected_var = 12
     init( )  {
	     B_Private_var = A_Public_var
     }
     override func A_open_func()  {
	    return 222
     }
     func B_open_func(){
        return 1
     }
}


let testA = A()

let testB01:B = B()
let testB01_func = testB01.A_open_func()
let testB02:A = B()
let testB02_func = testB02.A_open_func()


