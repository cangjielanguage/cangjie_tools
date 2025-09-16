// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class C56{
    var p:CString
    init(s:String){
        p = CString(s)
        println(s)
    }
}
var s1 = CString("fff")
