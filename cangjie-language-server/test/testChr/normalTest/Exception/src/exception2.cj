// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func exception2(){
    let arrayTest: RefArray<Int32> = RefArray<Int32>([0, 1, 2])
    try {
        let lastElement = arrayTest[3]
    } catch (e : err) {
        print("catch an exception!"+ e.getMessage())
    }
}
