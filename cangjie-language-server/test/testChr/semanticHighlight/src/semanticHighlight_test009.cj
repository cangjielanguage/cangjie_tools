// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

enum TimeUnit09 {
    Year | Month | Day | Hour
}

func semanticHighlight_test009(): unit {
    var a = TimeUnit09.Day
    var b:TimeUnit09 = .Day
}
