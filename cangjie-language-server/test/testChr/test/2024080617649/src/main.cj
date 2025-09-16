// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package demo

public class Student {
    public let name = 1
}

public class A {
    public func height() {
    }
}

public func foo(a: (Student) -> Unit): A {
    A()
}

func test() {
    foo { stu: Student => stu.name }.height()
}

