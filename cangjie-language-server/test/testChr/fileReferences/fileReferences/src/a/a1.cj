// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos_app_cangjie_entry.a
import ohos_app_cangjie_entry.b.B
import ohos_app_cangjie_entry.c.A
import ohos_app_cangjie_entry.b.I2
import ohos_app_cangjie_entry.b.I8
import ohos_app_cangjie_entry.b.s2
import ohos_app_cangjie_entry.b.Color2
import ohos_app_cangjie_entry.b.test4
import ohos_app_cangjie_entry.b.Rectangle1


class A1 <: testclass {}

interface I <: I1 {}

func test(a: I64) {}

class A2 <: B {}

interface I3 <: I2 {}

func test3(a: I8) {}

main() {
    let r = Rectangle(1, 2)
    test1()
    let s1 = s
    let g = Color.RED
    let r2 = Rectangle1(1, 2)
    test4()
    let s3 = s2
    let g2 = Color2.RED
}

func test2() {
    var a = 10;
    var b = a
    var c = a
    println("test1");
}