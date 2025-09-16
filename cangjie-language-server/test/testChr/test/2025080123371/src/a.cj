// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public class C1{
    init(a:Int32,fn:(Int32)->Int32) { fn(a) }
    init(fn:(Int64)->Int64) {fn(1)}
    C1(a:Int32,fn:(Int32)->Int32,b:Int64) { fn(a) }

}
public interface C3{}
public struct C2 <: C3{}
let c2:C2 = C2()
let t1 = C1(1,{i=>i*i})
let t2 = C1({i=>i*i})
let t3 = C1(1,{i=>i*i},2)
let ada:C1 = C1(2,{i=>i*i})
func abc(){var a:Int64 = 1

}

extend C1{
    func a(){}
}
