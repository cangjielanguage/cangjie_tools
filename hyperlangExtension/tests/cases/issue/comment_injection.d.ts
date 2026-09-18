// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入漏洞修复验证
// 覆盖路径：type_conversion.cj toCJType 的 FIXME 注释
// 攻击载荷：字符串字面量类型中包含 */ 以尝试闭合块注释

// 漏洞1: 字符串字面量类型别名 → toCJType 的 /* FIXME: `${ty}` */ 注入
type EvilStringLiteral = "*/evil_code_in_typealias()/*";
type EvilStringLiteral2 = '*/inject_in_single_quote/*';
type EvilUnion = "*/break_out_union/*" | "normal";
type EvilPromise = Promise<"*/promise_inject/*">;

// 漏洞1b: 字符串字面量作为函数返回类型
type EvilFunc = () => "*/func_return_inject/*";
