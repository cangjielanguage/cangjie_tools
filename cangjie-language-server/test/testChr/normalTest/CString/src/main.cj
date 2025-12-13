// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package DotCompleteCString

main() {
    var s1 = unsafe {LibC.mallocCString("hello")}
    let b1 = s1.
}