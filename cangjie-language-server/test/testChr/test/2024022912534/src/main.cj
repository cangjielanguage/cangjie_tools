// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

class B {
    B(var name: String) {

    }

    func height(h: Int64) {

    }
}

class Student {
    let name = "aa"
}

func Column(f: (Student, Int64) -> B): B {
    B("a")
}

func render() {
    Column {
        stuuu: Student, idx: Int64 =>
        B(stuuu)
    }.height(1)
}
