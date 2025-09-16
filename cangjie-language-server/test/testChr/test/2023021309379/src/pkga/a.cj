// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package bb.pkga

public interface ExCollection<T> <: Collection<T> where T <: Comparable<T>{
    operator func [](arg1 : Int64) : T
    operator func [](arg1 : Int64,value!: T):Unit{}
}
