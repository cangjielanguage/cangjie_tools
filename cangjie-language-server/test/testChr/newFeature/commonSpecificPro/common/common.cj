// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

@NonProduct
features {}
package lib


public common func message(): Unit

public common func message1(): Unit

public common func name333() {
    message()
    message1()
}

private let aaaa = 1


public func aaa() {
    message()
    AAA
}