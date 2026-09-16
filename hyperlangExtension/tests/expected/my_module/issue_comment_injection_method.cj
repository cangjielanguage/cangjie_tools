// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********OBJECT***********/
// 接口方法
/*interface ICommentInjection {
    methodWithStringParam(mode: "*\/interface_method_inject/\*"): void
    methodWithLineComment(): "*\/line_comment_return/\*"
    }*/

public open class ICommentInjection {
    protected ICommentInjection(var arkts_object: JSObject) { }
    
    
    /**
     * @brief methodWithStringParam(mode: "*\/interface_method_inject/\*"): void
     */
    public func methodWithStringParam(mode: JSValue/* FIXME: `"*\/interface_method_inject/\*"` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "methodWithStringParam", { ctx =>[mode] })
    }
    /**
	 * @brief methodWithLineComment(): "*\/line_comment_return/\*"
	 */
	// 普通行注释包含 */
    public func methodWithLineComment(): JSValue/* FIXME: `"*\/line_comment_return/\*"` */ {
        jsObjApiCall < JSValue/* FIXME: `"*\/line_comment_return/\*"` */ >( arkts_object, "methodWithLineComment", EMPTY_ARG) { ctx, info => info }
    }
    
    public open func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public static func fromJSValue(_: JSContext, input: JSValue): ICommentInjection {
        ICommentInjection(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief setMode(mode: "*\/evil_in_func_param()/\*"): void
	 */
	// 测试用例：注释注入漏洞修复验证
// 覆盖路径：mergeCommentToken 的 brief 参数（来自函数签名）
// 攻击载荷：函数参数类型为字符串字面量，其中包含 */ 以尝试在 @brief 注释中注入
// 漏洞2: 函数参数含字符串字面量类型 → mergeCommentToken 的 @brief 注入
public func setMode(mode: JSValue/* FIXME: `"*\/evil_in_func_param()/\*"` */): Unit {
    hmsGlobalApiCall < Unit >( "my_module_comment_injection_method", "setMode", { ctx =>[mode] })
}

/**
	 * @brief setConfig(key: "*\/key_inject/\*", value: "*\/value_inject/\*"): "*\/return_inject/\*"
	 */
	// 多参数场景
public func setConfig(key: JSValue/* FIXME: `"*\/key_inject/\*"` */, value: JSValue/* FIXME: `"*\/value_inject/\*"` */): JSValue/* FIXME: `"*\/return_inject/\*"` */ {
    hmsGlobalApiCall < JSValue/* FIXME: `"*\/return_inject/\*"` */ >( "my_module_comment_injection_method", "setConfig", { ctx =>[key, value] }) { ctx, info => info }
}

// 带注释的函数
/**
 * @brief withSinceComment(param: "*\/since_inject/\*"): void
 */
public func withSinceComment(param: JSValue/* FIXME: `"*\/since_inject/\*"` */): Unit {
    hmsGlobalApiCall < Unit >( "my_module_comment_injection_method", "withSinceComment", { ctx =>[param] })
}


