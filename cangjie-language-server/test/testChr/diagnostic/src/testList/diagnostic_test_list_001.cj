// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package testList
open class List_A{}

class List_B <:List_A{}

class List_C{}

struct List_D {
} 

 func TEST_LIST1(){
    let emptyList : List<int64> = [] // empty List<int64>
    let list_int = [1, 2, 3, 3, 2, 1] // list_int = [1, 2, 3, 3, 2, 1]
    let list_exp = [1 + 3, 2 + 3, 3 + 3] // list_exp = [4, 5, 6]
    list_exp.isEmpty()
    let list_spread1 = [1, 2, 3, *emptyList, 3, 2, 1] // list_spread1 = [1, 2, 3, 3,2, 1]
    let list_spread2 = [1, 2, 3, *list_int, 3, 2, 1] // list_spread2 = [1, 2, 3, 4, 5, 6, 3, 2, 1]
    let list_str = ["a", "b", "c"]
    let list_float = [4.0, 5.0, 6.0]
    let list_error_1 = [1, 2, 3, *list_float] // error: type mismatch
    let list_error_2 = [1.1, 2.2, 3.3, *list_str] // error: type mismatch

    let list_spread3 = [*[[1,2,3]]] // list7 = [[1,2,3]]
    let list_spread4 = [*[[1,2,3]],[4,5],[6,7,8]] // list8 = [[1,2,3],[4,5],[6,7,8]]
    let list_error_3 = [**[[1,2,3]],4,5,6] // syntax error


    let listZero: List<int32> = [] // empty List
    let listOne: List<int32> = [1, 2, 3, 4]
    var listTwo: List<int64> = [5, 6, 7]
    var list_initialization: List<float32>
    list_initialization = [3.14, 4.14, 5.14] // initialization

    var listFour = ["A", "B", "C"] // The type of listFour is inferred to be List<String>.
    var listFive = listFour // The type of listFive is inferred to be List<float32>.

    var list_class = [List_A(), List_B()] // The type of listSix is inferred to be List<A>.
    var list_class2 = [List_A(), List_C()] //The type of listSix is inferred to be List<object>.
    var list_class3 = [List_A(), List_D()] //error
    let nestedList = [[1, 2, 3, 4], [5, 6, 7]] // The type of nestedList is inferred to be List<List<int64>>.
}
