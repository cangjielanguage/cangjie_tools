// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入 — 块注释（多行注释）载体
// 说明：TS 扫描器保证块注释内不可能出现 */（会提前终止注释），
//       因此块注释载体仅测试正常渲染 + 行注释中的 */ 载荷。
//       注：块注释内 /* 在 Cangjie 中会开启嵌套层级，透传路径未转义，
//       故透传路径的块注释内不放 /*；no-init-const 路径会转义故保留。
// 覆盖路径：
//   - trans_object.cj generate()/genPropDeclare → 原始注释透传（Token(COMMENT)）
//   - trans_enum.cj generate() → 原始注释透传
//   - hle.cj transConst（有初始化值）/ transType → 原始注释透传
//   - analysis.js visitVariable1（无初始化值）→ JS 侧重包裹转义

// B1: interface 前块注释
/* B1 block interface comment */
export interface IBlockInjection {
    // B2: interface 属性前块注释（interface 属性注释当前被丢弃，锁定该行为）
    /* B2 block prop comment */
    blockProp: string;

    // B3: interface 方法前块注释（mergeCommentToken 拆分后由注释自身 */ 闭合）
    /* B3 block method comment */
    blockMethod(mode: string): void;
}

// B4: class 前块注释 + class 属性/构造函数/方法前块注释
/* B4 block class comment */
export declare class BlockClass {
    /* B4 block class prop comment */
    classBlockProp: number;

    /* B4 block constructor comment */
    constructor();

    /* B4 block class method comment */
    classBlockMethod(): void;
}

// B5: enum 前块注释（成员注释当前被丢弃）
/* B5 block enum comment */
export declare enum BlockEnum {
    /* B5 block enum member comment */
    BlockMember = 0,
}

// B6: type alias 前块注释
/* B6 block typealias comment */
export type BlockTypeAlias = string

// B7: const 有初始化值前块注释（原始透传路径）
// B7 line comment with */ payload
/* B7 block const with init comment */
export declare const BLOCK_CONST_WITH_INIT = 1

// B8: const 无初始化值前块注释（JS 侧重包裹转义路径，含 /* 载荷验证转义）
/* B8 block const no init comment with /* opener */
export declare const BLOCK_CONST_NO_INIT: string

// B9: function 前块注释
/* B9 block function comment */
export declare function blockFunc(): void;

// B10: 多行块注释（跨行，含 /* opener 载荷，无初始化值 const 重包裹路径）
/* B10 multi-line block comment
 * line two with /* opener payload
 * line three
 */
export declare const BLOCK_MULTI_LINE_NO_INIT: string
