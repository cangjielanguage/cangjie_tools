// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

// 行注释包含 */ — 类型别名，走 transType 路径
// evil_type_alias_inject() */
public type EvilTypeAliasWithComment = String


// 测试用例：注释注入漏洞修复验证
// 覆盖路径：
//   - hle.cj transConst（有初始化值）→ escapeRawComment
//   - analysis.js visitVariable1（无初始化值）→ JS 侧转义
//   - hle.cj transType → escapeRawComment
// 攻击载荷：d.ts 行注释中包含 */ 以尝试在重包裹块注释时注入
// 行注释包含 */ — 有初始化值，走 escapeRawComment 路径
// evil_const_with_value_inject() */
public const EVIL_CONST_WITH_VALUE = "safe_value"
/*
// 行注释包含 *\/ — 无初始化值，走 JS 侧 escape 路径
// evil_const_no_init_inject() *\/
public const EVIL_CONST_NO_INIT = 0/* FIXME: Initialization is required */
*/
// 行注释包含 */ — 另一个有值常量
// evil_line_comment_inject() */
public const EVIL_CONST_LINE_COMMENT = 42

