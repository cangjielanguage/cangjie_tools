// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
declare function testGeneric<T>(t: T): T;

declare function testMultiGenericT<T, M>(t: T, m: M): T;

declare function testMultiGenericM<T, M>(t: T, m: M): M;

declare function testNoGeneric(t: T): T;
 
declare function init(type: string, data: any);
 
declare function getObject(extra: number, info: string);