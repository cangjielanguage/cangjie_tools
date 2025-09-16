// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

var c: Int32 = 0

class R{
  protected func a(){
    c = 10
  }
}

let b = R().a


func faa(): Int64{
  let d = R().a

  R().a()
	b() //类似lambda表达式
	return Int64(c - 10)
}



class R2{
  protected func a1(){
    c = 10
  }
}
