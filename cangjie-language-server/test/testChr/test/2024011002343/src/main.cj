// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

from std import collection.HashMap
class CC{
    private static let _type = 0
    public static prop TYPE:Int64 {
        get() {
            _type
        }
    }
}

type TA = CC
type AA<T> = Array<T>
var mm = AA<TA>()
