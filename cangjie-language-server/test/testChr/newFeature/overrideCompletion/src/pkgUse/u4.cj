// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package overrideCompletion.pkgUse

import overrideCompletion.pkg1.*

open class C3<V> <: C2<V> & I1<V> {
    open public func fabni1(): Unit {

    }

    open public func fabni2(a: V): V {
        return a
    }

    open public func fabni3(a: V): V {
        return a
    }

    open protected func fabnc4() {

    }  
}

class C4<T1> <: C3<T1> {

    fabn

}

class C5<T> <: C3<T> {

    protected func fabn

}