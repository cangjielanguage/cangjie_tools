// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

export declare class AnonymousClass {
    f1(p: {n: number}): {n: number};
    f2(n: number): Promise<string>;
    f3(n: number): Promise<{
        msg: string;
    }>;
    f4(): Promise<{
        msg: string;
        data: string;
        code?: undefined;
    } | {
        code: number;
        msg: string;
        data?: undefined;
    }>;
}