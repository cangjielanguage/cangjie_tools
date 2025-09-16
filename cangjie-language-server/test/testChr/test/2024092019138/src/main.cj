// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test2024092019138

@Deprecated
open class Clazz {
    @Deprecated
    public var z: Int64 = 10

    @Deprecated
    public static var c: Int64 = 10
}

main(): Int64 {
    let clazz = Clazz.c
    return 0
}
