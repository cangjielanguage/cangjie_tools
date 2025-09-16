// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

func foo() : Int64 {
    return 0
}

let foo_spwn : Future<Int64> = spawn {
    foo()
}

func main() :Int64 {
    foo_spwn.
    return 0
}
