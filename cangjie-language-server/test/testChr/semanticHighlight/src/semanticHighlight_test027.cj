// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func semanticHighlight_test027_1(a:int64): int16 {
    return a
}

func semanticHighlight_test026(): int64 {
    var a:int64 = semanticHighlight_test027_1(1)
    return a
}
