// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testMap

open class Map_A{}
class Map_B <:Map_A{}
func TEST_MAP(){
    let aaa:Map<int32, int32>
    aaa.size()
    let mapZero: Map<int32, char> = #{} // empty Map

    let mapOne: Map<int32, String> = #{1: "Aa", 2: "Bb", 3: "Cc", 1: "fir"}
    // mapOne = #{1 : "Aa", 2 : "Bb", 3 : "Cc"}, as duplicated key and its value '1 : "fir"' is removed.
    var mapTwo = #{4: "Dd", 5: "Ee", 6: "Ff"} // The type of mapTwo is inferred to be Map<int64, String>.
    func foo() {
    var mapThree: Map<int64, char>
    mapThree = #{7: 'g', 8: 'h', 9: 'i'} // initialization
    mapThree.size()

    var mapFour = #{Map_A(): Map_A(), Map_A(): Map_A()} // The type of mapFour is inferred to be Map<A, B>.

    var mapFive = #{1..10: 1, 1..0: 2} // The type of mapFive is inferred to be Set<Range<Int64>>.

    var mapSix = #{1..10: 1: 100 , 1..10: 2: 200} // The type of mapSix is inferred to be Map<Range<Int64>, int32>.

    let nestedMap = #{#{1: "Aa", 2: "Bb", 3: "Cc"}: #{7: 'g', 8: 'h', 9: 'i'}}
    // The type of nestedMap is inferred to be Map<Map<Int64, String>, Map<Int64, Rune>>.
    let mapSeven = #{1: "A", 2: "B", 3: "C"} // The type of mapSeven is inferred to be Map<Int64, String>.
    mapSeven.size()
    //for item in mapFour { // The type of item is Int64*String.
    let mapZero: Map<int32, int32>
}
