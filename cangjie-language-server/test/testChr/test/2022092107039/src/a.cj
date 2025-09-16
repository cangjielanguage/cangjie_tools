// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func f_1(x:Int32):Float32 { Float32(x) }
func g_1(x:Float32):Int32 { Int32(x) }

var fg = f_1 ~> g_1
var fg2 = g_1 ~> f_
