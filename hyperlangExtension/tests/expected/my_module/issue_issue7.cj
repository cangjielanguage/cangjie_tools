// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType0 {
    | STRING(String)
    | NUMBER(Float64)
    | BOOLEAN(Bool)
    | UNDEFINED
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case STRING(x) => context.string(x).toJSValue()
            case NUMBER(x) => context.number(x).toJSValue()
            case BOOLEAN(x) => context.boolean(x).toJSValue()
            case UNDEFINED => context.undefined().toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        } 
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        }  
        if (input.isBoolean()) {
            return BOOLEAN(Bool.fromJSValue(context, input))
        } 
        if (input.isUndefined()) {
            return UNDEFINED
        }
        throw Exception("unsupport type!")
    }
}


/*class ExportKeyword DeclareKeyword AppSettings {
    getInstance(): AppSettings
    getSetting(key: string): String | number | boolean | undefined
    }*/

public class AppSettings {
    
    protected AppSettings(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief getInstance(): AppSettings
	 */
    public static func getInstance(): AppSettings {
        jsObjApiCall < AppSettings >(getClassConstructorObj("my_module_issue7", "AppSettings"),  "getInstance", emptyArg) {
            ctx, info => AppSettings.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief getSetting(key: string): String | number | boolean | undefined
	 */
    public func getSetting(key: String): AutoGenType0 {
        jsObjApiCall < AutoGenType0 >( arkts_object, "getSetting", { ctx =>[key.toJSValue(ctx)] }) {
            ctx, info => AutoGenType0.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): AppSettings {
        AppSettings(input.asObject())
    }
}

