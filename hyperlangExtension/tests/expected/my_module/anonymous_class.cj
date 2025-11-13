// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*

public enum AutoGenType5 {
    | AUTOGENTYPE3(AutoGenType3)
    | AUTOGENTYPE4(AutoGenType4)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case AUTOGENTYPE3(x) => x.toJSValue(context)
            case AUTOGENTYPE4(x) => x.toJSValue(context)
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType5 {
        
        throw Exception("unsupport type!")
    }
}


/*interface AutoGenType0 {
    n: number;
    }*/

public open class AutoGenType0 {
    
    protected AutoGenType0(public var n: Float64) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = n.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType0 {
        let obj = input.asObject()
        AutoGenType0(
        Float64.fromJSValue(context, obj["n"])
        )
    }
    
}

/*interface AutoGenType1 {
    n: number;
    }*/

public open class AutoGenType1 {
    
    protected AutoGenType1(public var n: Float64) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = n.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType1 {
        let obj = input.asObject()
        AutoGenType1(
        Float64.fromJSValue(context, obj["n"])
        )
    }
    
}

/*interface AutoGenType2 {
    msg: string;
    }*/

public open class AutoGenType2 {
    
    protected AutoGenType2(public var msg: String) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = msg.toJSValue(context)
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType2 {
        let obj = input.asObject()
        AutoGenType2(
        String.fromJSValue(context, obj["msg"])
        )
    }
    
}

/*interface AutoGenType3 {
    msg: string;
    data: string;
    code?: undefined;
    }*/

public open class AutoGenType3 {
    
    protected AutoGenType3(public var msg: String,
    public var data: String,
    public var code!: Option<JSValue/* FIXME: `undefined` */> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = msg.toJSValue(context)
        obj["data"] = data.toJSValue(context)
        if(let Some(v) <- code) {
            obj["code"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3 {
        let obj = input.asObject()
        AutoGenType3(
        String.fromJSValue(context, obj["msg"]),
        String.fromJSValue(context, obj["data"]),
        code: obj["code"]
        )
    }
    
}

/*interface AutoGenType4 {
    code: number;
    msg: string;
    data?: undefined;
    }*/

public open class AutoGenType4 {
    
    protected AutoGenType4(public var code: Float64,
    public var msg: String,
    public var data!: Option<JSValue/* FIXME: `undefined` */> = None) { }
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["code"] = code.toJSValue(context)
        obj["msg"] = msg.toJSValue(context)
        if(let Some(v) <- data) {
            obj["data"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4 {
        let obj = input.asObject()
        AutoGenType4(
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
    public func f1(p: AutoGenType0): AutoGenType1 {
        jsObjApiCall < AutoGenType1 >( arkts_object, "f1", { ctx =>[p.toJSValue(ctx)] }) {
            ctx, info => AutoGenType1.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief f2(n: number): Promise<String>
	 */
    public func f2(callback: AsyncCallback < JSValue/* FIXME: `Promise<string>` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<string>` */ >( arkts_object, "f2", callback, { ctx =>[n.toJSValue(ctx)] }) {
            ctx, info => info
        }
    }
    /**
	 * @brief f3(n: number): Promise<{
        msg: String;
    }>
	 */
    public func f3(callback: AsyncCallback < JSValue/* FIXME: `Promise<AutoGenType2>` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<AutoGenType2>` */ >( arkts_object, "f3", callback, { ctx =>[n.toJSValue(ctx)] }) {
            ctx, info => info
        }
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
    public func f4(callback: AsyncCallback < JSValue/* FIXME: `Promise<AutoGenType5>` */ >): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<AutoGenType5>` */ >( arkts_object, "f4", callback, emptyArg) {
            ctx, info => info
        }
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
     static func fromJSValue(context: JSContext, input: JSValue): AnonymousClass {
        AnonymousClass(input.asObject())
    }
}

