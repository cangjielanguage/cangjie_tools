// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
import { a } from '@umeng/common';
import buffer from '@ohos.buffer';
import { e } from "../g/h";

import { MyStringEnum, MyNumericEnum } from './exportAlias'; // Type Imports
declare const value1: MyStringEnum;
declare const value2: MyNumericEnum;

import * as Inheritances from './inheritances'; // Module Import
declare function createSub(): Inheritances.SubClass;

import { ExportedInterface } from './exportAlias'; // Module Augmentation
declare module './exportAlias' {
    interface ExportedInterface {
        myOption?: string;
    }
}