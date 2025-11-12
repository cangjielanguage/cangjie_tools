// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


type TB230<V = string, W extends 'abc' | 'def' = 'abc'> = (arg: V) => W;
type TB300 = Promise<number>;
type TB310 = Promise<boolean>[];