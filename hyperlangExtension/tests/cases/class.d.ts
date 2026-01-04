// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.


export declare class TDDiagBuilder {
    static readonly PLATFORM_OA = 0;
    static readonly PLATFORM_PRO = 3;
    appId: string;
    appKey: string;
    appVersion: string;
    platform: number;
    loggerAdapter: LoggerAdapter | null | undefined;
    logUploadListener?: LogUploadListener;
    initiativeUploadWhiteListTags?: Array<string>;
    xgTrafficQuota: number;
    platform: string | null;
    crashListener: ICrashListener | null;
    constructor();
}

export declare class d10 {
    private constructor();
    static getInstance(): d10;
    putSync(key: string, value: number | string | boolean | Array<number> | Array<string> | Array<boolean>): d10;
}