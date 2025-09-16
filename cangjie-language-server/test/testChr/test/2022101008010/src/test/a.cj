// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test

public struct CCC<T,U> {
    public var a:T
    public var b:U
    public init(a:T,b:U){
         this.a = a
         this.b =b
    }
}
