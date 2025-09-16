// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


interface Eq<TTTT> {

    func eq(other:TTTT): Bool

}

interface Ord<Tsss> where Tsss <: Eq<Tsss> {
    func lt(other: Tsss): Bool
}

func foo<ssss2s>(a: ssss2s) where ssss2s <: Ord< > {
    a.lt(a)
}

func foo2<ssss2s>(a: ssss2s) where ssss2s <: Ord<ssss2s> {
    a.
    a.lt(a)
}

