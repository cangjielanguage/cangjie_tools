// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
declare function f(): void;

// export statements may occur before definition
export {
    f as aliasedFunc,
    anotherFunc,
    MyInterface,
    I2 as ExportedInterface,
    MyClass,
    C1 as ExportedClass,
    MY_VALUE,
    V1 as EXPORTED_VALUE,
    MyStringEnum,
    E1 as MyNumericEnum,
}

declare function anotherFunc(): void;

interface MyInterface {}

interface I2 {}

export interface I3 {}

class MyClass {}

class C1 {}

const MY_VALUE = 123;

const V1 = "abc";

enum MyStringEnum { A = "aaa" }

enum E1 { A = 111 }
