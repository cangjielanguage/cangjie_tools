// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType5InanonymousClass {
    | AutoGenType5InanonymousClassAUTOGENTYPE3INANONYMOUSCLASS(AutoGenType3InanonymousClass)
    | AutoGenType5InanonymousClassAUTOGENTYPE4INANONYMOUSCLASS(AutoGenType4InanonymousClass)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case AutoGenType5InanonymousClassAUTOGENTYPE3INANONYMOUSCLASS(x) => x.toJSValue(context)
            case AutoGenType5InanonymousClassAUTOGENTYPE4INANONYMOUSCLASS(x) => x.toJSValue(context)
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType5InanonymousClass {
        
        throw Exception("unsupport type!")
    }
}


/*interface AutoGenType0InanonymousClass {
    n: number;
    }*/

public open class AutoGenType0InanonymousClass {
    public var n: Float64
    protected init(n: Float64) {
        this.n = n
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = n.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0InanonymousClass {
        let obj = input.asObject()
        AutoGenType0InanonymousClass(
        Float64.fromJSValue(context, obj["n"])
        )
    }
}


/*interface AutoGenType1InanonymousClass {
    n: number;
    }*/

public open class AutoGenType1InanonymousClass {
    public var n: Float64
    protected init(n: Float64) {
        this.n = n
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = n.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1InanonymousClass {
        let obj = input.asObject()
        AutoGenType1InanonymousClass(
        Float64.fromJSValue(context, obj["n"])
        )
    }
}


/*interface AutoGenType2InanonymousClass {
    msg: string;
    }*/

public open class AutoGenType2InanonymousClass {
    public var msg: String
    protected init(msg: String) {
        this.msg = msg
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = msg.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2InanonymousClass {
        let obj = input.asObject()
        AutoGenType2InanonymousClass(
        String.fromJSValue(context, obj["msg"])
        )
    }
}


/*interface AutoGenType3InanonymousClass {
    msg: string;
    data: string;
    code?: undefined;
    }*/

public open class AutoGenType3InanonymousClass {
    public var msg: String
    public var data: String
    public var code: Option<JSValue/* FIXME: `undefined` */> = None
    protected init(
    msg: String,
    data: String,
    code!: Option<JSValue/* FIXME: `undefined` */> = None) {
        this.msg = msg
        this.data = data
        this.code = code
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = msg.toJSValue(context)
        obj["data"] = data.toJSValue(context)
        if (let Some(v) <- code) {
            obj["code"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3InanonymousClass {
        let obj = input.asObject()
        AutoGenType3InanonymousClass(
        String.fromJSValue(context, obj["msg"]),
        String.fromJSValue(context, obj["data"]),
        code: obj["code"]
        )
    }
}


/*interface AutoGenType4InanonymousClass {
    code: number;
    msg: string;
    data?: undefined;
    }*/

public open class AutoGenType4InanonymousClass {
    public var code: Float64
    public var msg: String
    public var data: Option<JSValue/* FIXME: `undefined` */> = None
    protected init(
    code: Float64,
    msg: String,
    data!: Option<JSValue/* FIXME: `undefined` */> = None) {
        this.code = code
        this.msg = msg
        this.data = data
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["code"] = code.toJSValue(context)
        obj["msg"] = msg.toJSValue(context)
        if (let Some(v) <- data) {
            obj["data"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4InanonymousClass {
        let obj = input.asObject()
        AutoGenType4InanonymousClass(
        Float64.fromJSValue(context, obj["code"]),
        String.fromJSValue(context, obj["msg"]),
        data: obj["data"]
        )
    }
}


/*class ExportKeyword DeclareKeyword AnonymousClass {
    f1(p: {n: number}): {n: number}
    f2(n: number): Promise<String>
    f3(n: number): Promise<{
        msg: String;
    }>
    f4(): Promise<{
        msg: String;
        data: String;
        code?: undefined;
    } | {
        code: number;
        msg: String;
        data?: undefined;
    }>
    }*/

public class AnonymousClass {
    protected AnonymousClass(var arkts_object: JSObject) { }
    
    
    /**
	 * @brief f1(p: {n: number}): {n: number}
	 */
    public func f1(p: AutoGenType0InanonymousClass): AutoGenType1InanonymousClass {
        jsObjApiCall < AutoGenType1InanonymousClass >( arkts_object, "f1", { ctx =>[p.toJSValue(ctx)] }) { ctx, info => AutoGenType1InanonymousClass.fromJSValue(ctx, info) }
    }
    /**
	 * @brief f2(n: number): Promise<String>
	 */
    public func f2(callback: AsyncCallback < JSValue/* FIXME: `Promise<string>` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<string>` */ >( arkts_object, "f2", callback, { ctx =>[n.toJSValue(ctx)] }) { ctx, info => info }
    }
    /**
	 * @brief f3(n: number): Promise<{
        msg: String;
    }>
	 */
    public func f3(callback: AsyncCallback < JSValue/* FIXME: `Promise<AutoGenType2InanonymousClass>` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<AutoGenType2InanonymousClass>` */ >( arkts_object, "f3", callback, { ctx =>[n.toJSValue(ctx)] }) { ctx, info => info }
    }
    /**
	 * @brief f4(): Promise<{
        msg: String;
        data: String;
        code?: undefined;
    } | {
        code: number;
        msg: String;
        data?: undefined;
    }>
	 */
    public func f4(callback: AsyncCallback < JSValue/* FIXME: `Promise<AutoGenType5InanonymousClass>` */ >): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<AutoGenType5InanonymousClass>` */ >( arkts_object, "f4", callback, EMPTY_ARG) { ctx, info => info }
    }
    
    func toJSValue(_: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(_: JSContext, input: JSValue): AnonymousClass {
        AnonymousClass(input.asObject())
    }
}

