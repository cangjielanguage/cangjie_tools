// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
import {p29} from "./superInterface"

export declare interface SuperInterface {
    p: number;
}

export declare class SubInterface implements SuperInterface {
    p: number;
    p1: number;
}

export declare abstract class SuperClass {
    p: number;
}

export declare class SubClass extends SuperClass {
    p: number;
}

export declare class SuperClass1 {
    p: number;
}

export declare class SubClass1 extends SuperClass1 {
    p: number;
}

interface A {
    p: number;
}

interface B extends A {
    p1: number
}

interface C {
    f(): void
}

interface D extends C {
}

interface E extends A {
}

interface F extends C {
    g(): void
}

export interface BuglyLogAdapter extends p29 {
}
