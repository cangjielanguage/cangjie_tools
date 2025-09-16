// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package demo.a

public let illegal = 1

open class C1 {
    func init<T,V>(a:T,b:V) {  
        var a01=a
        var b01=b
        var aaa=10
    }
    func init(T)(a:T) {
        var a01=a
        var aaa=10
    }
}
