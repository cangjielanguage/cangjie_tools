// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package test2025051513422
import std.sync.ReentrantReadWriteMutex

main(): Int64 {
    let mxt = ReentrantReadWriteMutex()
    synchronized(mxt){}
    return 0
}
