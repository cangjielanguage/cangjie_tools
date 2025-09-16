// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

enum TimeUnit {
    Year
}

func test1() {
    var time = TimeUnit.YEAR
    var res: Int32 = match (time) {
        case TimeUnit.Year => 0
        case ddd => 1
    }
}
