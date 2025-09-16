// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func TEST_SET(){
    let aaa: List<int32>
    aaa.size()
    //let setZero: Set<int32> = #{} // empty Set
    //let setOne: Set<int32> = #{5, 6, 7, 5} // setOne = #{5,6,7}, as duplicated element '5' is removed.
    //var setTwo = #{1.1, 2.1, 3.1, 4.1} // The type of setTwo is inferred to be Set<float64>.
    setTwo.size()
    setZero.size()
    var setThree = #{FFF(), FFF()} // The type of setThree is inferred to be Set<Object>.

    var setStr = #{"sa ","sa","sa","sa"}
    let nestedSet = #{#{1, 2, 3, 4}, #{5, 6, 7}} // The type of nestedSet is inferred to be Set<Set<int64>>.
    nestedSet.size()
    nestedSet.contains(#{1, 2, 3, 4})
    nestedSet.size()
    nestedSet.isEmpty()
    nestedSet.iterator()
    nestedSet.size()
    let nestedSet2 = #{#{1, 2, 3, 4}, #{5, 6, 0.1}} //error
    var XXX: int32 = 0.1
    nestedSet.isEmpty()
}
