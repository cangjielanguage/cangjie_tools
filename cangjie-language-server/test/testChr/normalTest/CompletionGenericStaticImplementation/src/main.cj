// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package completion_generic_static

public interface StableParsable<T> {
    static func stableParse(value: String): T
}

extend Int64 <: StableParsable<Int64> {
    public static func stableParse(data: String): Int64 { 0 }
}

main(): Int64 {
    Int64.stableParse
    0
}
