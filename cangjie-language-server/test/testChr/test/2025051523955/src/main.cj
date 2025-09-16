// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test2025051523955

foreign {
    func test(): CPointer<Unit>
}

main(): Int64 {
    unsafe {
        let p1 = test()
        if (p1) {

        }
    }
    return 0
}
