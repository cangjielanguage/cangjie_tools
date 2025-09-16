// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class Solution1 {
    func twoSum(nums: Array<Int64>, target: Int64): Array<Int64> {
        //  遍历数组中两两相加的所有可能，以数组形式返回相加结果等于目标值的两个数
        for (i in 0..nums.size()) {
            for (j in (i + 1)..nums.size()) {
                if (nums[i] + nums[j] == target) {
                    return [i, j]
                }
            }
        }
        return []
    }
}
