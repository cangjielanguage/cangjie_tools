// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package bb.pkgb

import bb.pkga.*

public class Sort<T, U> where T <: ExCollection<U> {
    public static func bubbleSort(arr: T) {
        let length = arr.size
        for (i in 0..length - 1) {
            for (j in 0..length - i - 1) {
                if (arr[j] > arr[j + 1]) {
                    let tmp = arr[j]
                    arr[j] = arr[j + 1]
                    arr[j + 1] = tmp
                }
            }
        }
        return arr
    }
}

main(){}
