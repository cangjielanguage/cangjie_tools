// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

export class ArgumentMatchers {
    static any;
    static anyString;
    static anyBoolean;
    static anyNumber;
    static anyObj;
    static anyFunction;
    static matchRegexs(Regex: RegExp): void
}

declare interface when {
    afterReturn(value: any): any
    afterReturnNothing(): undefined
    afterAction(action: any): any
    afterThrow(e_msg: string): string
    (argMatchers?: any): when;
}

export const when: when;

export interface VerificationMode {
    times(count: Number): void
    never(): void
    once(): void
    atLeast(count: Number): void
    atMost(count: Number): void
}

export class MockKit {
    constructor()
    mockFunc(obj: Object, func: Function): Function
    mockObject(obj: Object): Object
    verify(methodName: String, argsArray: Array<any>): VerificationMode
    ignoreMock(obj: Object, func: Function): void
    clear(obj: Object): void
    clearAll(): void
}

export declare function MockSetup(
    target: Object,
    propertyName: string | Symbol,
    descriptor: TypedPropertyDescriptor<() => void>
): void;