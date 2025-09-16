// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testInterface

interface I1 {
    func I1_01( ) : Int8
    func I1_02( ) : Int16
    static func I1_03( ) : Unit{}
}
interface I2 <: I1 {
    func I2_01( ) : Int8
    func I2_02( ) : Int16
    static func I2_03( ) : Unit{}
}

class  AA{
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

class I1_A <: AA & I2{
    var A_Public_var=222
    var I1_A_x = 0
    init() {
	     I1_A_x = A_Public_var
    }
    func I1_01( ) : Int8{
        return 1
    }
    func I1_02( ) : Int16{
         return 1
     }

    func I2_01( ) : Int8{
        return 2
    }
    func I2_02( ) : Int16{
         return 2
     }
}

let x01_I1_A = I1_A()
let test01 = x01_I1_A.I1_A_x
let x02_I2:I2 = I1_A()
let test02 = x02_I2.I1_02()
let x03_AA:AA = I1_A()
let test03 = x03_AA.A_open_func()

