// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package InterpolatedString

func test44(a: Bool): Int {
    return 3
}


func call44() {
    var a = "xxxx${test44()}xxxx"
    var b = 'xxxx${test44(true)}xx'
    var c = """
    ${test44(false)}
    """

}
