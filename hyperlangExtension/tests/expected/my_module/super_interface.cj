// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

/***********OBJECT***********/

/*interface ExportKeyword p29 {
    debug(arg: string): void
    info(arg: string): void
    warn(arg: string): void
    error(arg: string): void
    fatal(arg: string): void
    }*/

public open class p29 {
    
    protected p29(public var arkts_object: JSObject) { }
    
    
    /**
	 * @brief debug(arg: string): void
	 */
    public func debug(arg: String): Unit {
        jsObjApiCall < Unit >( arkts_object, "debug", { ctx =>[arg.toJSValue(ctx)] })
    }
    /**
	 * @brief info(arg: string): void
	 */
    public func info(arg: String): Unit {
        jsObjApiCall < Unit >( arkts_object, "info", { ctx =>[arg.toJSValue(ctx)] })
    }
    /**
	 * @brief warn(arg: string): void
	 */
    public func warn(arg: String): Unit {
        jsObjApiCall < Unit >( arkts_object, "warn", { ctx =>[arg.toJSValue(ctx)] })
    }
    /**
	 * @brief error(arg: string): void
	 */
    public func error(arg: String): Unit {
        jsObjApiCall < Unit >( arkts_object, "error", { ctx =>[arg.toJSValue(ctx)] })
    }
    /**
	 * @brief fatal(arg: string): void
	 */
    public func fatal(arg: String): Unit {
        jsObjApiCall < Unit >( arkts_object, "fatal", { ctx =>[arg.toJSValue(ctx)] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    public open  static func fromJSValue(context: JSContext, input: JSValue): p29 {
        p29(input.asObject())
    }
}

