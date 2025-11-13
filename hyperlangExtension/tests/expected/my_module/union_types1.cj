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
    | BOOLEAN(Bool)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case BOOLEAN(x) => context.boolean(x).toJSValue()
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        
        if (input.isBoolean()) {
            return BOOLEAN(Bool.fromJSValue(context, input))
        } 
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType1 {
    | STRING(String)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case STRING(x) => context.string(x).toJSValue()
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1 {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        } 
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}

// type alias
public enum a1 {
    | STRING(String)
    | NUMBER(Float64)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case STRING(x) => context.string(x).toJSValue()
            case NUMBER(x) => context.number(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): a1 {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        } 
        if(input.isNumber()) {
            return NUMBER(Float64.fromJSValue(context, input))
        } 
        throw Exception("unsupport type!")
    }
}



/*class ExportKeyword A {
    a: a1;
    b: boolean | String;
    test1(a:a1): String|number
    test2(a: string|number): number
    }*/

public class A {
    
    protected A(var arkts_object: JSObject) { }
    
    
    public mut prop a: a1 {
        get() {
            checkThreadAndCall < a1 >(getMainContext()) {
                ctx: JSContext => a1.fromJSValue(ctx, arkts_object["a"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["a"] = v.toJSValue(ctx)
            }
        }
        
    }
    // property
    public mut prop b: AutoGenType0 {
        get() {
            checkThreadAndCall < AutoGenType0 >(getMainContext()) {
                ctx: JSContext => AutoGenType0.fromJSValue(ctx, arkts_object["b"])
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => arkts_object["b"] = v.toJSValue(ctx)
            }
        }
        
    }
    
    /**
	 * @brief test1(a:a1): String|number
	 */
	// member function
    public func test1(a: a1): AutoGenType1 {
        jsObjApiCall < AutoGenType1 >( arkts_object, "test1", { ctx =>[a.toJSValue(ctx)] }) {
            ctx, info => AutoGenType1.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief test2(a: string|number): number
	 */
    public func test2(a: String): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "test2", { ctx =>[a.toJSValue(ctx)] })
    }
    /**
	 * @brief test2(a: string|number): number
	 */
    public func test2(a: Float64): Float64 {
        jsObjApiCall < Float64 >( arkts_object, "test2", { ctx =>[a.toJSValue(ctx)] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): A {
        A(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief test(a:a1): String|number
	 */
	// global func
public func test(a: a1): AutoGenType1 {
    hmsGlobalApiCall < AutoGenType1 >( "my_module_unionTypes1", "test", { ctx =>[a.toJSValue(ctx)] }) {
        ctx, info => AutoGenType1.fromJSValue(ctx, info)
    }
}


