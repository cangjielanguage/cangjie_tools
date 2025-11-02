// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

interface UserProfile {
    id?: number|string;
    array: (number | string)[]
    test:(num: number | string) => string
    test1:number|(boolean | string)[];
    test2:number| boolean[];
    name: string;
    address: {
      city: string;
      zipCode: string;
    };
}

