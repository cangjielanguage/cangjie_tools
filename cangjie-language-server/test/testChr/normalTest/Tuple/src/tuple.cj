// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


import test.*


func main(): Int64 {
    var pi = (3.14, "PIE")
    print("${pi[0]}")
    print("${pi[1]}")
    return 0
}


func multiValues(a: Int32, b: Int32) { (a + b, a - b) } // The type of the return value of the function multiValues is Int32*Int32.
func main1() {
    var (x, y) = multiValues(8,24) // Define an anonymous tuple who has two elements, i.e., x and y.
    print("${x}") // output: 32
    print("${y}") // output: -16
    return 0
}

func main2() {
    let tuplePIE = (3.14, "PIE")     // The type of tuplePIE is inferred to be Float64*String.
    var pointOne = (2.4, 3.5) 		// The type of pointOne is inferred to be Float64*Float64.
    var pointTwo = (2, 3, 4) 		// The type of pointTwo is inferred to be Int64*Int64*Int64.
    var pointThree = ((2, 3), 4) 	// The type of pointThree is inferred to be (Int64*Int64)*Int64.

    // 重命名，选中高亮，查找引用
    var a = tuplePIE[0]
    var b = tuplePIE[1]
    var c = tuple2
    var d = tuple3
}
