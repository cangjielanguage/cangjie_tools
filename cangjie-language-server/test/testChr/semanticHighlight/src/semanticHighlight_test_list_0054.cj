// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

open class List_A{}

class List_B <:A{}
 func TEST_LIST(){
    let emptyList : List<int64> = [] // empty List<int64>

    let list0 = [1, 2, 3, 3, 2, 1] // list0 = [1, 2, 3, 3, 2, 1]
    let list1 = [1 + 3, 2 + 3, 3 + 3] // list1 = [4, 5, 6]
    list1.size()
    let emptyList11 : List<int64> = []
    emptyList11.size()
    let list2 = [1, 2, 3, *emptyList, 3, 2, 1] // list0 = [1, 2, 3, 3,2, 1]
    let list3 = [4, 5, 6]
    let list4 = [1, 2, 3, *list1, 3, 2, 1] // list2 = [1, 2, 3, 4, 5, 6, 3, 2, 1]

    let list5 = [4.0, 5.0, 6.0]
    let list6 = [1, 2, 3, *list5] // error: type mismatch
    let list7 = ["a", "b", "c"]
    let list8 = [1.1, 2.2, 3.3, *list7] // error: type mismatch

    let list9 = [*[[1,2,3]]] // list7 = [[1,2,3]]
    let list10 = [*[[1,2,3]],[4,5],[6,7,8]] // list8 = [[1,2,3],[4,5],[6,7,8]]
    let list11 = [**[[1,2,3]],4,5,6] // syntax error


    let listZero: List<int32> = [] // empty List
    let listOne: List<int32> = [1, 2, 3, 4]
    var listTwo: List<int64> = [5, 6, 7]
    func foo() {
    var listThree: List<float32>
    listThree = [3.14, 4.14, 5.14] // initialization

    var listFour = ["A", "B", "C"] // The type of listFour is inferred to be List<String>.
    var listFive = listThree // The type of listFive is inferred to be List<float32>.

    var listSix = [List_A(), List_B()] // The type of listSix is inferred to be List<A>.
    let nestedList = [[1, 2, 3, 4], [5, 6, 7]] // The type of nestedList is inferred to be List<List<int64>>.
}
