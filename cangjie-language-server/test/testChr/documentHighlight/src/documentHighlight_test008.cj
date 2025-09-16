// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func assignment_test(): int64 {
    var x:int32 = 5
    var y:int32 = 10
    x = y
    x += y
    x -= y 
    x *= y
    x /= y 
    x %= y
    x = 5
    x **= 2
    x <<= 1
    x >>= 2
    x &= y
    x ^= y
    x |= y
    x++
    y-- 
    return 1
}
