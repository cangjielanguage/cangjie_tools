// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

// 测试用例：注释注入漏洞修复验证
// 覆盖路径：type_conversion.cj toCJType 的 FIXME 注释
// 攻击载荷：字符串字面量类型中包含 */ 以尝试闭合块注释
// 漏洞1: 字符串字面量类型别名 → toCJType 的 /* FIXME: `${ty}` */ 注入
public type EvilStringLiteral = JSValue/* FIXME: `"*\/evil_code_in_typealias()/\*"` */


public type EvilStringLiteral2 = JSValue/* FIXME: `"*\/inject_in_single_quote/\*"` */


public enum EvilUnion {
    | LITERAL___BREAK_OUT_UNION__
    | NORMAL
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case LITERAL___BREAK_OUT_UNION__ => context.string("*/break_out_union/*").toJSValue()
            case NORMAL => context.string("normal").toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): EvilUnion {
        
        throw Exception("unsupport type!")
    }
}


public type EvilPromise = JSValue/* FIXME: `Promise<"*\/promise_inject/\*">` */

// 漏洞1b: 字符串字面量作为函数返回类型
public type EvilFunc = () -> JSValue/* FIXME: `"*\/func_return_inject/\*"` */


