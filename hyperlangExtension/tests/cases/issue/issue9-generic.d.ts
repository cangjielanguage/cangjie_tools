// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

declare function testGeneric<T>(t:T): T | string

declare function testGeneric1<T, U>(t:T): T | U | string

declare class A<T, U> {
    f1(t:T): T | string

    f2(t:T): T | U | string
}

declare interface B<W, M> {
    f1<T>(t:T): T | W | string

    f2<T>(t:T): T | M | string
}