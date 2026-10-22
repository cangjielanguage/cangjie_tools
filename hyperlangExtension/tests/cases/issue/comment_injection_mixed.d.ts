// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入 — 组合注释（多条注释 join 合并）载体
// 覆盖路径：
//   - analysis.js getLeadingCommentRanges join('\n') 合并 + 无初始化值重包裹转义
//   - 有初始化值 const / interface 成员 → 原始注释透传
//   - trans_method.cj mergeCommentToken 的 since 标签拆分逻辑

// M1: 连续多个单行注释（无初始化值 const → JS 侧 join 合并 + 重包裹转义路径）
// M1 first line with */
// M1 second line with /*
export declare const MIXED_MULTI_LINE_NO_INIT: string

// M2: 块注释 + 行注释混合（无初始化值 const → JS 侧重包裹转义路径）
/* M2 block part */
// M2 line part with */
export declare const MIXED_BLOCK_LINE_NO_INIT: string

// M3: 行注释 + 块注释混合（有初始化值 const → 原始透传路径，注释原样拼接）
// M3 line part with */
/* M3 block part */
export declare const MIXED_LINE_BLOCK_WITH_INIT = 42

// M4: JSDoc 后跟 since 标签行注释（mergeCommentToken 在首个 since 标签处拆分，
//     拆分点后缀被吞入已开启的行注释中）
/**
 * M4 doc part
 */
// @since 1.0.0
export declare function mixedSinceFunc(param: string): void;

// M5: since 标签出现在描述文本中（首个标签命中拆分点，@brief 后缀被吞入行注释）
// M5 mentions @since in description text */
export declare function sinceInTextFunc(): void;

// M6: since 标签位于块注释内（拆分后由原注释自身的 */ 闭合，结构安全）
/* M6 block mentions @since inside */
export declare function sinceInBlockFunc(): void;

// M7: namespace 前注释 + namespace 内成员注释（namespace 级注释当前被丢弃，
//     内部成员注释进入无初始化值重包裹路径）
// M7 namespace comment */
declare namespace MixedNS {
    // M7 ns member comment */
    export const nsMixedVar: string;
}

// M8: interface 方法 JSDoc + 行注释混合 + 参数/返回值字面量载荷
export interface IMixedMethod {
    /**
     * M8 doc comment
     */
    // M8 line comment with */
    mixedMethod(mode: "*/mixed_param/*"): "*/mixed_return/*";
}
