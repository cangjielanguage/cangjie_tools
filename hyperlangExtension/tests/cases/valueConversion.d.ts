// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
import { MyStringEnum, MyNumericEnum } from './exportAlias';

interface I1 {
    a: number;
    b?: string;
    c: Uint8Array;
    d?: ArrayBuffer;
    e: Float32Array;
    f?: boolean;
    g: Record<string, Uint8Array>;
    h: MyStringEnum;
    i: bigint;
}

class C1 {
    static sa = 123;
    static sb: string;
    static sc?: string = "abc"
    static readonly sd = 999
    static readonly se: string = "zzz"
    va: number = 234;
    vb: Float32Array;
    vc?: MyNumericEnum;
    vd? = "def";
    ve?: bigint[];
}
