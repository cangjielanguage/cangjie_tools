// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

var i8Number1: Int8 = 127
var i16Number1: Int16 = 0
func funcTest3():Unit{
    var i8Number1 = Int16(i16Number1) //err
    i8Number1 = i16Number1
    i8Number1 = Int16(i16Number1) + i16Number1 + Int16(i16Number1) //err
}
