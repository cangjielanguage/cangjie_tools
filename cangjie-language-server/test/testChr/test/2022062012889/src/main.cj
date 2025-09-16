// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

public class CC {
    CC(x: Int64) {}
    static let mm = 8
    var nn = 7
}

type TypeCC1 = CC
type TypeCC2 = TypeCC1

enum EE {
    EEA | EEB | EEC

    static func fun1() {}
    func fun2() {}
}

type TypeEE1 = EE
type TypeEE2 = TypeEE1

main() {
    var a = 1
    TypeCC1
    a = 2
    TypeCC2
    a = 3
    TypeEE1
    a = 4
    TypeEE2
    a = 5
    var cc: TypeCC2
    a = 6
    var ee: TypeEE2
    0
}

