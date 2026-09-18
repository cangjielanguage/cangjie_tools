// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.
// 测试用例：注释注入漏洞修复验证
// 覆盖路径：mergeCommentToken 的 brief 参数（来自函数签名）
// 攻击载荷：函数参数类型为字符串字面量，其中包含 */ 以尝试在 @brief 注释中注入

// 漏洞2: 函数参数含字符串字面量类型 → mergeCommentToken 的 @brief 注入
declare function setMode(mode: "*/evil_in_func_param()/*"): void;

// 多参数场景
declare function setConfig(key: "*/key_inject/*", value: "*/value_inject/*"): "*/return_inject/*";

// 带注释的函数
/**
 * @since 8.0.0
 */
declare function withSinceComment(param: "*/since_inject/*"): void;

// 接口方法
interface ICommentInjection {
    /**
     * @since 1.0.0
     */
    methodWithStringParam(mode: "*/interface_method_inject/*"): void;

    // 普通行注释包含 */
    methodWithLineComment(): "*/line_comment_return/*";
}
