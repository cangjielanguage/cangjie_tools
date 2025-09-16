// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func semanticHighlight_test025_1(): int64 {
    return 1
}

func semanticHighlight_test025(): int64 {
    var index11: int64 = 100;
    let testarr = 1...5;
    for index1 in testarr {
        index11 = index1 + 5;
    }
    while index11 < 20 {
        print("The value of index is $(index11)");
        index11 = index11 + 1;
    }
    return semanticHighlight_test025_1(1)
}
