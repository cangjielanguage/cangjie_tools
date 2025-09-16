// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

//1
let aa = ({xy:Int32 => 6*xy})

//2
public class C1 {
    init(a: Int32, fn: (Int32) -> Int32) {
        fn(a)
    }
    init(fn: (Int64) -> Int64) {
        fn(1)
    }
    C1(a: Int32, fn: (Int32) -> Int32, b: Int64) {
        fn(a)
    }
}

let t1 = C1(1, {xy => xy})
let t2 = C1(1){xy => 2*xy}
