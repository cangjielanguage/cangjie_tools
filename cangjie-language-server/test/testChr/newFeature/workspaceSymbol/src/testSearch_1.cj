// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

external var testToplevelVarable: (Int64, Int64) -> Int64 = {a, b => return a + b}

type testTypealiasOfTuple = Array<Array<Int64>> * Array<Array<Int64>>
type testTypealiasOfLambda = (Int64, Int64) -> Int64;

