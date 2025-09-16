// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Solution9 {
    func isPalindrome(x: Int64): Bool {
        let str: String = x.toString()

        //  判断数字是否小于 0，若小于 0 则直接判断为 false
        if (x < 0) {
            return false
        } else {
            if (str != str.reverse()) {return false}
            return true
        }
    }
}
