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
    | NUMBER(Float64)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case NUMBER(x) => context.number(x).toJSValue()
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        }  
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}


/*class ExportKeyword DeclareKeyword C {
    f(p:T): number | String
    }*/

public class C {
    
    protected C(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief f(p:T): number | String
	 */
    public func f < T >(p: T): AutoGenType0 where T <: JSInteropType<T> {
        jsObjApiCall < AutoGenType0 >( arkts_object, "f", { ctx =>[p.toJSValue(ctx)] }) {
            ctx, info => AutoGenType0.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): C {
        C(input.asObject())
    }
}

