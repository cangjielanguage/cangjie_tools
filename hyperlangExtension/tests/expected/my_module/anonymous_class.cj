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
    | _MSG_STRING_DATA_STRING_CODE__UNDEFINED_(JSValue/* FIXME: `{ msg: string; data: string; code?: undefined }` */)
    | _CODE_NUMBER_MSG_STRING_DATA__UNDEFINED_(JSValue/* FIXME: `{ code: number; msg: string; data?: undefined }` */)
    
    public func toJSValue(context: JSContext): JSValue {
        match(this) {
            case _MSG_STRING_DATA_STRING_CODE__UNDEFINED_(x) => x
            case _CODE_NUMBER_MSG_STRING_DATA__UNDEFINED_(x) => x
        }
    }
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType5 {
        
    }
}


/*interface AutoGenType0 {
    n: ;
    }*/

public open class AutoGenType0 {
    
    protected AutoGenType0(public var : Float64) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = .toJSValue(context)
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
    n: ;
    }*/

public open class AutoGenType1 {
    
    protected AutoGenType1(public var : Float64) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["n"] = .toJSValue(context)
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
    msg: ;
    }*/

public open class AutoGenType2 {
    
    protected AutoGenType2(public var : String) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = .toJSValue(context)
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
    msg: ;
    data: ;
    code?: ;
    }*/

public open class AutoGenType3 {
    
    protected AutoGenType3(public var : String,
    public var : String,
    public var !: Option<JSValue/* FIXME: `undefined` */> = None) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["msg"] = .toJSValue(context)
        obj["data"] = .toJSValue(context)
        if(let Some(v) <- ) {
            obj["code"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType3 {
        let obj = input.asObject()
        AutoGenType3(
        String.fromJSValue(context, obj["msg"]),
        String.fromJSValue(context, obj["data"]),
        : obj["code"]
        )
    }
    
}

/*interface AutoGenType4 {
    code: ;
    msg: ;
    data?: ;
    }*/

public open class AutoGenType4 {
    
    protected AutoGenType4(public var : Float64,
    public var : String,
    public var !: Option<JSValue/* FIXME: `undefined` */> = None) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["code"] = .toJSValue(context)
        obj["msg"] = .toJSValue(context)
        if(let Some(v) <- ) {
            obj["data"] = v
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): AutoGenType4 {
        let obj = input.asObject()
        AutoGenType4(
        Float64.fromJSValue(context, obj["code"]),
        String.fromJSValue(context, obj["msg"]),
        : obj["data"]
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
    
    protected AnonymousClass(var arkts_object: JSObject) {}
    
    
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
    public func f3(callback: AsyncCallback < JSValue/* FIXME: `Promise<{ msg: string }>` */ >, n: Float64): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<{ msg: string }>` */ >( arkts_object, "f3", callback, { ctx =>[n.toJSValue(ctx)] }) {
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
    public func f4(callback: AsyncCallback < JSValue/* FIXME: `Promise<{ msg: string; data: string; code?: undefined } | { code: number; msg: string; data?: undefined }>` */ >): Unit {
        jsObjApiCallPromise < JSValue/* FIXME: `Promise<{ msg: string; data: string; code?: undefined } | { code: number; msg: string; data?: undefined }>` */ >( arkts_object, "f4", callback, emptyArg) {
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

