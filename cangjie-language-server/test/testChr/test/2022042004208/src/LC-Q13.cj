// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

from std import collection.*

class Solution13 {
    func romanToInt(s: String): Int64 {
        //  创建一个罗马字母与对应数字的 HashMap
        let romanToIntMap = HashMap<String, Int64>([("I", 1), ("V", 5), ("X", 10), ("L", 50), ("C", 100), ("D", 500), ("M", 1000)])
        var num = romanToIntMap[s.substring(s.size() - 1, 1)]

        //  与下一个数字比较，若大于则相加，否则相减
        for (i in 0..s.size() - 1) {
            var mapVol: Int64 = romanToIntMap[s.substring(i, 1)]
            if (mapVol >= romanToIntMap[s.substring(i + 1, 1)]) {
                num += mapVol
            } else {
                num -= mapVol
            }
        }

        return num
    }
}
