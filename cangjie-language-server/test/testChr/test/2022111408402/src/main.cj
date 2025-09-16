// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface AAAAA {}

class AAAA1<T> <: AAAAA {}

class BBBB {}

main() {
    let aa1 = AAAA1<Int64>()
    let aa2 = BBBB()
    0
}
