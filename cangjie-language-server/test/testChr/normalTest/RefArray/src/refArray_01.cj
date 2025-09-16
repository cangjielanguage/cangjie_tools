// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class AAAA{}
let external_emptyArrr =  RefArray<Int64>()
func testArray(){

    let emptyArr1 = RefArray<Int64>() // create an empty array whose type is RefArray<Int64>
    let emptyArr2 = RefArray<String>() // create an empty array whose type is  RefArray<String>
    let classsAAA = RefArray<AAAA>()

    let array1 = RefArray<Int64>([0, 1, 2, 3, 4, 5]) // 'array1' has 6 elememts: 0, 1, 2, 3, 4, 5
    let array_str = RefArray<String>(["0", "1"])

    //var array2 = RefArray<RefArray<Int64>>([[0, 1, 2], [3, 4, 5]]) // 'array2' is a two-dimensional array 暂时不支持多维RefArray字面量初始化



    let array1s = RefArray<Int64>([0, 1, 2, 3, 4, 5]) // 'array1' has 6 elememts: 0, 1, 2, 3, 4, 5

    let array3 = RefArray<Int64>(3) { i => i * 2 } // 'array3' has 3 elememts: 0, 2, 4
    //let array4 = RefArray<RefArray<Int64>>(2) { i => RefArray<Int64>(2) {j => i + j} } // array4 = RefArray<RefArray<Int64>>([[0, 1], [1, 2]])
    let array5 = RefArray<Int64>([0, 1])

    let element0 = array5[0] // element0 = 0
    array5[1] = 10 // change the value of the second element of 'array5' through index

    let array_error = RefArray<Int64>([0, 0.1])
    /*
    //暂不支持
    let array6 = RefArray<RefArray<Float64>>([[0.1, 0.2], [0.3, 0.4]])


    let element01 = array6[0][1] // element01 = 0.2
    array6[1][1] = 4.0 // change the value of the last element of 'array6' through index
    */

    let array9 = RefArray<Int64>([0, 1, 2, 3, 4, 5])
    
    func slicingTest() {
        array9[0..5] // RefArray<Int64>([0, 1, 2, 3, 4])
        array9[0..5:1] // RefArray<Int64>([0, 1, 2, 3, 4])
        array9[0..5:2] // RefArray<Int64>([0, 2, 4])
        array9[5..0:-1] // RefArray<Int64>([5, 4, 3, 2, 1])
        array9[5..0:-2] // RefArray<Int64>([5, 3, 1])
        array9[0..=5] // RefArray<Int64>([0, 1, 2, 3, 4, 5])
        array9[0..=5:1] // RefArray<Int64>([0, 1, 2, 3, 4, 5])
        array9[0..=5:2] // RefArray<Int64>([0, 2, 4])
        array9[5..=0:-1] // RefArray<Int64>([5, 4, 3, 2, 1, 0])
        array9[5..=0:-2] // RefArray<Int64>([5, 3, 1])
        array9[..4] // RefArray<Int64>([0, 1, 2, 3])
        array9[2..] // RefArray<Int64>([2, 3, 4, 5])
        array9[..] // RefArray<Int64>([0, 1, 2, 3, 4, 5])
        array9[..4:2] // error: the value of 'step' cannot be set.
        array9[2..:-2] // error: the value of 'step' cannot be set.
        array9[..:-1] // error: the value of 'step' cannot be set.
        array9[..=4] // RefArray<Int64>([0, 1, 2, 3, 4])
        array9[..=4:2] // error: the value of 'step' cannot be set.
        array9[0..5:-1] // RefArray<Int64>()
        array9[5..=0] // RefArray<Int64>()
        array9[..0] // RefArray<Int64>()
        array9[..=-1] // RefArray<Int64>()
    }
}

func accesss() {
    let array8 = RefArray<Int64>([1, 8, 0, 1, 0])
    for (num in array8) {
        print("${num}") // output: 18010
    }

    let array9 = RefArray<Int64>([0, 1, 2, 3, 4, 5])
    let size1 = array9.size() // size1 = 6
    let array10 = RefArray<RefArray<Int64>>([[0, 1, 2], [3, 4, 5]])
    let size2 = array10.size() // size2 = 2

}

