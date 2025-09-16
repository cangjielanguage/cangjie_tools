// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface AE1<T,R>{}
interface AE2 {}

class AE3<T,R> <: AE1<T, R> & AE1<Int64, R> & AE1<T, Bool> & AE2{}
