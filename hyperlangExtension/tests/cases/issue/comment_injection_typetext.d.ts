// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入 — 类型文本载荷变体（流入 toCJType FIXME 注释 / @brief 签名注释）
// 覆盖路径：
//   - type_conversion.cj toCJType 各 FIXME 分支（字面量/tuple/数组/泛型/联合）
//   - trans_method.cj mergeCommentToken @brief（origFuncSignature 原始签名透传）
//   - analysis.js serializeType 对各类节点的文本化

// T1: 纯 closer 载荷（*/）
type TextCloserOnly = "closer_only */";

// T2: 纯 opener 载荷（/*）
type TextOpenerOnly = "/* opener_only";

// T3: 重叠载荷（*/ 与 /* 共享中间字符 *，验证转义替换后不产生新注释定界符）
type TextOverlap = "*/*";

// T4: 模板字面量类型（载荷在解析阶段丢失，FIXME 显示 UnsupportedType，锁定该行为）
type TextTemplate = `*/template_payload/*`;

// T5: tuple 元素为字符串字面量
type TextTuple = [string, "*/tuple_payload/*"];

// T6: 数组元素为字符串字面量
type TextArray = ("*/array_payload/*")[];

// T7: 泛型参数为字符串字面量
type TextGeneric = Array<"*/generic_payload/*">;

// T8: interface 属性类型为字符串字面量（必选 + 可选，流入属性类型 FIXME）
export interface ITextProp {
    literalProp: "*/prop_payload/*";
    optionalLiteralProp?: "*/optional_payload/*";
}

// T9: 函数参数为 tuple（含字面量）、返回值为字面量（原始签名流入 @brief 注释）
export declare function textTupleFunc(t: [string, "*/func_tuple/*"]): "*/func_return/*";

// T10: 类型表达式尾部注释（成为下一声明的 leading comment，
//      进入无初始化值 const 重包裹转义路径）
type TextTrailingComment = string /* T10 trailing with /* opener */
export declare const TEXT_AFTER_TRAILING: string
