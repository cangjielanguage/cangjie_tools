// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入 — 语义标签（systemapi/deprecated/since/default）与值位置载荷
// 覆盖路径：
//   - util.cj checkAPIAndCommentOut → addComment 重包裹（整体转义）
//   - common.cj @default 提取 → Property.defValue 透传（Token(IDENTIFIER) 直接插入）
//   - common.cj isSafeDefaultValue → 未加引号且含注释定界符的值被拒绝，降级为无默认值
//   - trans_enum.cj 字符串枚举值 → quote() 自动加引号（载荷位于字符串字面量内）
// 注意：@default 未加引号的 */ 载荷曾经是代码注入漏洞，现已修复（isSafeDefaultValue
//       过滤 + 警告拒绝），S8 锁定拒绝降级行为，S4 锁定带引号值的透传行为

// S1: systemapi 标签 + 行注释载荷 + 字面量参数（整体被注释掉，重包裹转义）
// @systemapi
// S1 sysapi payload */
export declare function sysapiFunc(mode: "*/sysapi_param/*"): void;

// S2: deprecated 标签 + 载荷（整体被注释掉，重包裹转义；
//     interface 属性注释被丢弃，属性随类一起注释）
// @deprecated
export interface IDeprecatedInjection {
    // S2 dep prop payload */
    depProp: string;
}

// S3: systemapi 枚举（整体被注释掉，重包裹转义）
// @systemapi
export declare enum SysapiEnum {
    SysapiMember = 0,
}

// S4: default 标签带引号值（defValue 原样透传为字符串字面量，编译安全；
//     属性注释本身被 interface 路径丢弃，锁定 defValue 落点）
export interface IDefaultInjection {
    // @default "safe*/quoted/*default"
    defaultProp?: string;
}

// S8: default 标签未加引号载荷（值含 */ 无法成为合法表达式，且会在注释包裹
//     路径中提前闭合块注释构成注入；isSafeDefaultValue 拒绝透传并打印警告，
//     属性降级为 Option<T> 无默认值形态，锁定拒绝降级行为；
//     未加引号的合法字面量（浮点/布尔）不受过滤误杀，正常透传）
export interface IDefaultInjectionUnsafe {
    // @default */unquoted_evil_payload/*
    unsafeProp?: string;
    // @default 3.14
    safeNumericProp?: number;
    // @default true
    safeBoolProp?: boolean;
}

// S5: 字符串枚举成员单引号含载荷（quote() 自动加引号 → 载荷位于字符串字面量内，安全）
export declare enum SingleQuoteEnum {
    EvilSingle = '*/single_quote_payload/*',
    NormalSingle = 'ok'
}

// S6: 字符串枚举成员双引号含载荷（quote() 自动加引号，行为与 S5 一致）
export declare enum DoubleQuoteEnum {
    EvilDouble = "*/double_quote_payload/*",
    NormalDouble = "ok"
}

// S7: JSDoc since 标签正常形式 + 参数/返回值字面量载荷
//     （mergeCommentToken 在首个 since 标签处拆分）
/**
 * @since 8.0.0
 */
export declare function jsdocSinceFunc(mode: "*/jsdoc_param/*"): "*/jsdoc_return/*";
