// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

let setZero: Set<Int32> = #{} // empty Set
let setOne: Set<Int32> = #{5, 6, 7, 5} // setOne = #{5,6,7}, as duplicated element '5' is removed.
var setTwo = #{1.1, 2.1, 3.1, 4.1} // The type of setTwo is inferred to be Set<Float64>.
class Aset { }
class Bset { }
var setThree = #{Aset(), Bset()} // The type of setThree is inferred to be Set<Object>.
let nestedSet = #{#{1, 2, 3, 4}, #{5, 6, 7}} // The type of nestedSet is inferred to be Set<Set<Int64>>.

func settest123() {
    let intSet: Set<Int32> = #{0, 1, 2, 3, 4}
    for (item in intSet) {
        print("${item}") // output: 01234
    }
    return 0
}
