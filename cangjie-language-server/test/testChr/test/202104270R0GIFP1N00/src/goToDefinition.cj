// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

enum E{
    | f(Int64)
    | f(Float32,Float32)
    | f(Int32,Int32,Int32)
}

func f(a:Float16) {}
func f(a:Int64,b:Int64) {}

func g() {
    f(1.0)
    E.f(1)
    f(1,2)
    f(1,2,3)

}
