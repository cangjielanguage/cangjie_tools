// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func lambdaExpr() {
    let y = 14
    let unitFn = { =>
        if (y == 14) {
            println(y)
        } else {
            print(-1)
        }
    }
    let typeFn = { =>
        let x = y - 1
        x
    }
}
