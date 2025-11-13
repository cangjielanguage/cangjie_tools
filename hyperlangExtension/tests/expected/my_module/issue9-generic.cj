// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType0 < T > where T <: JSInteropType<T> {
    | T0(T)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case T0(x) => x.toJSValue(context)
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 < T > {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType1 < T, U > where T <: JSInteropType<T>, U <: JSInteropType<U> {
    | T0(T)
    | U0(U)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case T0(x) => x.toJSValue(context)
            case U0(x) => x.toJSValue(context)
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1 < T, U > {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType2 < T, W > where T <: JSInteropType<T>, W <: JSInteropType<W> {
    | T0(T)
    | W0(W)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case T0(x) => x.toJSValue(context)
            case W0(x) => x.toJSValue(context)
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2 < T, W > {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

public enum AutoGenType3 < T, M > where T <: JSInteropType<T>, M <: JSInteropType<M> {
    | T0(T)
    | M0(M)
    | STRING(String)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case T0(x) => x.toJSValue(context)
            case M0(x) => x.toJSValue(context)
            case STRING(x) => context.string(x).toJSValue()
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3 < T, M > {
        
        if(input.isString()) {
            return STRING(String.fromJSValue(context, input))
        }
        throw Exception("unsupport type!")
    }
}

/***********OBJECT***********/

/*interface DeclareKeyword B {
    f1(t:T): T | W | String
    f2(t:T): T | M | String
    }*/

public open class B<W, M> where W <: JSInteropType<W>, M <: JSInteropType<M> {
    
    protected B(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief f1(t:T): T | W | String
	 */
    public func f1 < T >(t: T): AutoGenType2< T, W > where T <: JSInteropType<T> {
        jsObjApiCall < AutoGenType2< T, W > >( arkts_object, "f1", { ctx =>[t.toJSValue(ctx)] }) {
            ctx, info => AutoGenType2< T, W >.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f2(t:T): T | M | String
	 */
    public func f2 < T >(t: T): AutoGenType3< T, M > where T <: JSInteropType<T> {
        jsObjApiCall < AutoGenType3< T, M > >( arkts_object, "f2", { ctx =>[t.toJSValue(ctx)] }) {
            ctx, info => AutoGenType3< T, M >.fromJSValue(ctx, info)
        }
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): B<W, M> {
        B(input.asObject())
    }
}

/*class DeclareKeyword A {
    f1(t:T): T | String
    f2(t:T): T | U | String
    }*/

public class A<T, U> where T <: JSInteropType<T>, U <: JSInteropType<U> {
    
    protected A(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief f1(t:T): T | String
	 */
    public func f1(t: T): AutoGenType0< T > {
        jsObjApiCall < AutoGenType0< T > >( arkts_object, "f1", { ctx =>[t.toJSValue(ctx)] }) {
            ctx, info => AutoGenType0< T >.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f2(t:T): T | U | String
	 */
    public func f2(t: T): AutoGenType1< T, U > {
        jsObjApiCall < AutoGenType1< T, U > >( arkts_object, "f2", { ctx =>[t.toJSValue(ctx)] }) {
            ctx, info => AutoGenType1< T, U >.fromJSValue(ctx, info)
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): A<T, U> {
        A(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief testGeneric(t:T): T | String
	 */
public func testGeneric < T >(t: T): AutoGenType0 where T <: JSInteropType<T> {
    hmsGlobalApiCall < AutoGenType0 >( "my_module_issue9-generic", "testGeneric", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => AutoGenType0.fromJSValue(ctx, info)
    }
}

/**
	 * @brief testGeneric1(t:T): T | U | String
	 */
public func testGeneric1 < T, U >(t: T): AutoGenType1 where T <: JSInteropType<T>, U <: JSInteropType<U> {
    hmsGlobalApiCall < AutoGenType1 >( "my_module_issue9-generic", "testGeneric1", { ctx =>[t.toJSValue(ctx)] }) {
        ctx, info => AutoGenType1.fromJSValue(ctx, info)
    }
}


