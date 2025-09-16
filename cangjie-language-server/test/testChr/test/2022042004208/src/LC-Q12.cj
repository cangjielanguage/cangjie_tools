// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Solution12 {
    func intToRoman(num: Int64): String {
        //  定义两个固定顺序的数组，两个数组的值一一对应
        let int: Array<Int64> = [1000, 900, 500, 400, 100, 90, 50, 40, 10, 9, 5, 4, 1]
        let roman: Array<String> = ["M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I"]
        var str: String = ""
        var tmp: Int64 = num

        //  将待转换的数字从大到小的顺序匹配
        for (i in 0..=12) {
            while (tmp >= int[i]) {
                str += roman[i]
                tmp -= int[i]
            }
        }

        return str
    }
}


