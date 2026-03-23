// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

@NonProduct
features {}
package lib.pkg


public common func message_pkg(): Unit

public common func message1_pkg(): Unit

public func addfun(a:Int8, b:Int8) {
    a + b
}

public common func message2_pkg(): Unit {
    addfun(1, 2)
}
