// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import std.collection.{ HashMap }

public type ErrorCode = Float64


/***********ENUM***********/

/* enum EventType {
	}*/
public enum EventType <: ToString & Equatable < EventType > {
    
    func get(): Int32 {
        match(this) {
        }
    }
    
    static func parse(val: Int32): EventType {
        match(val) {
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?EventType {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
        }
    }
    
    public override operator func ==(that: EventType): Bool {
        match((this, that)) {
            case _ => false
        }
    }
    
    public override operator func !=(that: EventType): Bool {
        !(this == that)
    }
}


/* enum ListenerStatusNumeric {
	 on,
	 off
	}*/
public enum ListenerStatusNumeric <: ToString & Equatable < ListenerStatusNumeric > {
    | on
    | off 

    func get(): Int32 {
        match(this) {
            case on => 0  //todo: please check the value
            case off => 1  //todo: please check the value
        }
    }
    
    static func parse(val: Int32): ListenerStatusNumeric {
        match(val) {
            case 0 => on  //todo: please check the value
            case 1 => off  //todo: please check the value
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?Int32): ?ListenerStatusNumeric {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        match(this) {
            case on => "on"
            case off => "off"
        }
    }
    
    public override operator func ==(that: ListenerStatusNumeric): Bool {
        match((this, that)) {
            case(on, on) => true
            case(off, off) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: ListenerStatusNumeric): Bool {
        !(this == that)
    }
}


/* enum ListenerStatusString {
	 on = ON,
	 off = OFF
	}*/
public enum ListenerStatusString <: ToString & Equatable < ListenerStatusString > {
    | on
    | off 

    func get(): String {
        match(this) {
            case on => "ON" 
            case off => "OFF" 
        }
    }
    
    static func parse(val: String): ListenerStatusString {
        match(val) {
            case "ON" => on 
            case "OFF" => off 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?ListenerStatusString {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: ListenerStatusString): Bool {
        match((this, that)) {
            case(on, on) => true
            case(off, off) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: ListenerStatusString): Bool {
        !(this == that)
    }
}


/***********OBJECT***********/

/*interface TestListener {
    onStart?: () => void;
    onDestroy?: () => void;
    onError?: (code: ErrorCode, msg: String) => void;
    onTouch?: () => void;
    onEvent?: (e: EventType) => void;
    }*/

public open class TestListener {
    
    protected TestListener(public var onStart!: Option<() -> Unit> = None,
    public var onDestroy!: Option<() -> Unit> = None,
    public var onError!: Option<(code: ErrorCode, msg: String) -> Unit> = None,
    public var onTouch!: Option<() -> Unit> = None,
    public var onEvent!: Option<(e: EventType) -> Unit> = None) {}
    
    
    public open func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        if(let Some(v) <- onStart) {
            obj["onStart"] = context.function({ ctx, _ =>
                v()
                ctx.undefined().toJSValue()
            }).toJSValue()
        }
        if(let Some(v) <- onDestroy) {
            obj["onDestroy"] = context.function({ ctx, _ =>
                v()
                ctx.undefined().toJSValue()
            }).toJSValue()
        }
        if(let Some(v) <- onError) {
            obj["onError"] = context.function({ ctx, info =>
                let p0 = ErrorCode.fromJSValue(ctx, info[0])
                let p1 = String.fromJSValue(ctx, info[1])
                v(p0, p1)
                ctx.undefined().toJSValue()
            }).toJSValue()
        }
        if(let Some(v) <- onTouch) {
            obj["onTouch"] = context.function({ ctx, _ =>
                v()
                ctx.undefined().toJSValue()
            }).toJSValue()
        }
        if(let Some(v) <- onEvent) {
            obj["onEvent"] = context.function({ ctx, info =>
                let p0 = EventType.parse(Int32.fromJSValue(ctx, info[0]))
                v(p0)
                ctx.undefined().toJSValue()
            }).toJSValue()
        }
        obj.toJSValue()
    }
    
    public static func fromJSValue(context: JSContext, input: JSValue): TestListener {
        let obj = input.asObject()
        TestListener(
        onStart: if(obj["onStart"].isUndefined()) {
            None
        } else {
            { =>
                checkThreadAndCall < Unit >(context, { _ =>
                    obj["onStart"].asFunction().call()
                })
            }
        },
        onDestroy: if(obj["onDestroy"].isUndefined()) {
            None
        } else {
            { =>
                checkThreadAndCall < Unit >(context, { _ =>
                    obj["onDestroy"].asFunction().call()
                })
            }
        },
        onError: if(obj["onError"].isUndefined()) {
            None
        } else {
            { code: ErrorCode, msg: String =>
                checkThreadAndCall < Unit >(context, { ctx =>
                    let arg0 = code.toJSValue(ctx)
                    let arg1 = msg.toJSValue(ctx)
                    obj["onError"].asFunction().call([arg0, arg1])
                })
            }
        },
        onTouch: if(obj["onTouch"].isUndefined()) {
            None
        } else {
            { =>
                checkThreadAndCall < Unit >(context, { _ =>
                    obj["onTouch"].asFunction().call()
                })
            }
        },
        onEvent: if(obj["onEvent"].isUndefined()) {
            None
        } else {
            { e: EventType =>
                checkThreadAndCall < Unit >(context, { ctx =>
                    let arg0 = e.get().toJSValue(ctx)
                    obj["onEvent"].asFunction().call([arg0])
                })
            }
        }
        )
    }
    
}

/*interface MyListener {
    on(key: string, cb: (r: Record<string, string>) => void): void
    off(key: string, cb: (r: Record<string, string>, t:number) => void): void
    }*/

public open class MyListener {
    let callbackManager = CallbackManager < String, JSValue >()
    
    
    protected MyListener(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief on(key: string, cb: (r: Record<string, string>) => void): void
	 */
    public func on(key: String, cb: Callback1Argument<JSValue/* FIXME: `Record` */>): Unit {
        let key = key.toString()
        if(callbackManager.findCallbackObject(key, cb).isSome()) {
            return
        }
        let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
            __ctx => __ctx.function {
                __ctx: JSContext, info: JSCallInfo =>
                let arg0 = Record<JSValue/* FIXME: `stringstring` */>.fromJSValue(__ctx, info[0])
                cb.invoke(arg0)
                __ctx.undefined().toJSValue()
            }.toJSValue()
        }
        callbackManager.put(key,(cb, jsCallback))
        jsObjApiCall < Unit >( arkts_object, "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
    }
    /**
	 * @brief off(key: string, cb: (r: Record<string, string>, t:number) => void): void
	 */
    public func off(key: String, cb: Callback2Argument<JSValue/* FIXME: `Record` */>): Unit {
        let __ctx = getMainContext()
        let key = key.toString()
        if(let Some(v) <- cb) {
            let jsCallback = callbackManager.findCallbackObject(key, v, remove: true)
            if(jsCallback.isNone()) {
                throw BusinessException(401, "Callback does not exist.")
            }
            jsObjApiCall < Unit >( arkts_object, "off", { __ctx =>[key.toJSValue(__ctx), jsCallback ?? __ctx.undefined().toJSValue()] })
        } else {
            if(callbackManager.isCallbackExist(key)) {
                callbackManager.clear(key)
                jsObjApiCall < Unit >( arkts_object, "off", { __ctx =>[key.toJSValue(__ctx)] })
            }
        }
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyListener {
        MyListener(input.asObject())
    }
}

/*interface MyListener2 {
    on(key: string, cb: (r: ListenerStatusNumeric) => void): void
    }*/

public open class MyListener2 {
    let callbackManager = CallbackManager < String, JSValue >()
    
    
    protected MyListener2(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief on(key: string, cb: (r: ListenerStatusNumeric) => void): void
	 */
    public func on(key: String, cb: Callback1Argument<ListenerStatusNumeric>): Unit {
        let key = key.toString()
        if(callbackManager.findCallbackObject(key, cb).isSome()) {
            return
        }
        let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
            __ctx => __ctx.function {
                __ctx: JSContext, info: JSCallInfo =>
                let arg0 = ListenerStatusNumeric.parse(Int32.fromJSValue(__ctx, info[0]))
                cb.invoke(arg0)
                __ctx.undefined().toJSValue()
            }.toJSValue()
        }
        callbackManager.put(key,(cb, jsCallback))
        jsObjApiCall < Unit >( arkts_object, "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyListener2 {
        MyListener2(input.asObject())
    }
}

/*interface MyListener3 {
    on(key: string, cb: (r: ListenerStatusString) => void): void
    }*/

public open class MyListener3 {
    let callbackManager = CallbackManager < String, JSValue >()
    
    
    protected MyListener3(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief on(key: string, cb: (r: ListenerStatusString) => void): void
	 */
    public func on(key: String, cb: Callback1Argument<ListenerStatusString>): Unit {
        let key = key.toString()
        if(callbackManager.findCallbackObject(key, cb).isSome()) {
            return
        }
        let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
            __ctx => __ctx.function {
                __ctx: JSContext, info: JSCallInfo =>
                let arg0 = ListenerStatusString.parse(String.fromJSValue(__ctx, info[0]))
                cb.invoke(arg0)
                __ctx.undefined().toJSValue()
            }.toJSValue()
        }
        callbackManager.put(key,(cb, jsCallback))
        jsObjApiCall < Unit >( arkts_object, "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyListener3 {
        MyListener3(input.asObject())
    }
}

/*interface MyListener5 {
    on(key: string, cb: Callback<number>): void
    }*/

public open class MyListener5 {
    let callbackManager = CallbackManager < String, JSValue >()
    
    
    protected MyListener5(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief on(key: string, cb: Callback<number>): void
	 */
    public func on(key: String, cb: Callback1Argument<Float64>): Unit {
        let key = key.toString()
        if(callbackManager.findCallbackObject(key, cb).isSome()) {
            return
        }
        let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
            __ctx => __ctx.function {
                __ctx: JSContext, info: JSCallInfo =>
                let arg0 = Float64.fromJSValue(__ctx, info[0])
                cb.invoke(arg0)
                __ctx.undefined().toJSValue()
            }.toJSValue()
        }
        callbackManager.put(key,(cb, jsCallback))
        jsObjApiCall < Unit >( arkts_object, "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyListener5 {
        MyListener5(input.asObject())
    }
}

/*class MyListener4 {
    on(key: string, cb: (r: ListenerStatusString) => void): void
    }*/

public class MyListener4 {
    let callbackManager = CallbackManager < String, JSValue >()
    
    
    protected MyListener4(var arkts_object: JSObject) {}
    
    
    /**
	 * @brief on(key: string, cb: (r: ListenerStatusString) => void): void
	 */
    public static func on(key: String, cb: Callback1Argument<ListenerStatusString>): Unit {
        let key = key.toString()
        if(callbackManager.findCallbackObject(key, cb).isSome()) {
            return
        }
        let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
            __ctx => __ctx.function {
                __ctx: JSContext, info: JSCallInfo =>
                let arg0 = ListenerStatusString.parse(String.fromJSValue(__ctx, info[0]))
                cb.invoke(arg0)
                __ctx.undefined().toJSValue()
            }.toJSValue()
        }
        callbackManager.put(key,(cb, jsCallback))
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_functionTypes", "MyListener4"), arkts_object, "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MyListener4 {
        MyListener4(input.asObject())
    }
}

let callbackManager = CallbackManager < String, JSValue >()

/***********METHOD***********/
/**
	 * @brief on(key: string, cb: (r: Record<string, string>, option, t?:number) => void): void
	 */
public func on(key: String, cb: Callback3Argument<JSValue/* FIXME: `Record` */>): Unit {
    let key = key.toString()
    if(callbackManager.findCallbackObject(key, cb).isSome()) {
        return
    }
    let jsCallback = checkThreadAndCall < JSValue >(getMainContext()) {
        __ctx => __ctx.function {
            __ctx: JSContext, info: JSCallInfo =>
            let arg0 = Record<JSValue/* FIXME: `stringstring` */>.fromJSValue(__ctx, info[0])
            let arg1 = JSValue/* FIXME: `option` */.fromJSValue(__ctx, info[1])
            let arg2 = Float64.fromJSValue(__ctx, info[2])
            cb.invoke(arg0, arg1, arg2)
            __ctx.undefined().toJSValue()
        }.toJSValue()
    }
    callbackManager.put(key,(cb, jsCallback))
    hmsGlobalApiCall < Unit >( "my_module_functionTypes", "on", { __ctx =>[key.toJSValue(__ctx), jsCallback] })
}

/**
	 * @brief off(key: string, cb?: (r: Record<string, string>, t:number) => void): void
	 */
public func off(key: String, cb!:?Callback2Argument<JSValue/* FIXME: `Record` */> = None): Unit {
    let __ctx = getMainContext()
    let key = key.toString()
    if(let Some(v) <- cb) {
        let jsCallback = callbackManager.findCallbackObject(key, v, remove: true)
        if(jsCallback.isNone()) {
            throw BusinessException(401, "Callback does not exist.")
        }
        hmsGlobalApiCall < Unit >("my_module_functionTypes", "off", { __ctx =>[key.toJSValue(__ctx), jsCallback ?? __ctx.undefined().toJSValue()] })
    } else {
        if(callbackManager.isCallbackExist(key)) {
            callbackManager.clear(key)
            hmsGlobalApiCall < Unit >("my_module_functionTypes", "off", { __ctx =>[key.toJSValue(__ctx)] })
        }
    }
}


