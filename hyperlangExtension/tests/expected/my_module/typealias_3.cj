// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType0Intypealias_3 {
    | STRING(String)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case STRING(x) => context.string(x).toJSValue()
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0Intypealias_3 {
        
        if (input.isString()) {
            return STRING(String.fromJSValue(context, input))
        } 
        if (input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}


public enum MyUnion8 {
    | AUTOGENTYPE0INTYPEALIAS_3__(Array<AutoGenType0Intypealias_3>)
    | LITERAL__KEY_STRING_(JSValue/* FIXME: `{ key: string }` */)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case AUTOGENTYPE0INTYPEALIAS_3__(x) => toJSArray < AutoGenType0Intypealias_3 >(context, x, { ctx: JSContext, val: AutoGenType0Intypealias_3 => val.toJSValue(ctx) })
            case LITERAL__KEY_STRING_(x) => x
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): MyUnion8 {
        
        throw Exception("unsupport type!")
    }
}


