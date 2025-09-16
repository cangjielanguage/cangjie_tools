// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.



enum TimeUnit1 {
    Year | Month | Day | Hour
}

enum TimeUnit2 {
    | Year(Float32)
    | Month(Float32, Float32)
    | Day(Float32, Float32, Float32)
    | Hour(Float32, Float32, Float32, Float32)
}

enum TimeUnit5 {
    | Year(Int32)
    | Month(Int32, Float32)
    | Day(Int32, Float32, Float32)
    | Hour(Int32, Float32, Float32, Float32)
    | Twounit(TimeUnit5, TimeUnit5)
} 
enum Foo111 {
    A | B | C
    func f1() {      
        f2(this)
    }
    static func f2(v: Foo111) {
        match (v) {
            case $A => 0
            case $B => 1
            case $C => 2
        }
    }
    prop item: Int64 {
        get() {
            f1()
        }
    }
}


enum Option<T> { Some(T) | None }
