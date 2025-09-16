// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package demo

struct RecordData<T> {
    var a: T
    public init(x: T){
        a = x
    }
}

type RD<T> = RecordData<T>

func testjj(): Int64 {
    var struct1: RD<Int32> = RecordData<Int32>(2)
    return 1
}

enum Color<T> {
    Red(T)
}

type Food<T> = Color<T>
