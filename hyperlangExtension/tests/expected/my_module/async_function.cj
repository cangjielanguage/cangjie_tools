// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
import tpc.AbilityKit.common
/***********ENUM***********/

/* enum Colors {
	 Red = RED,
	 Green = GREEN,
	 Blue = BLUE
	}*/
public enum Colors <: ToString & Equatable < Colors > {
    | Red
    | Green
    | Blue 

    func get(): String {
        match(this) {
            case Red => "RED" 
            case Green => "GREEN" 
            case Blue => "BLUE" 
        }
    }
    
    static func parse(val: String): Colors {
        match(val) {
            case "RED" => Red 
            case "GREEN" => Green 
            case "BLUE" => Blue 
            case _ => throw IllegalArgumentException("unknown value ${val}")
        }
    }
    
    static func tryParse(val: ?String): ?Colors {
        match(val) {
            case Some(v) => parse(v)
            case None => None
        }
    }
    
    public func toString(): String {
        get()
    }
    
    public override operator func ==(that: Colors): Bool {
        match((this, that)) {
            case(Red, Red) => true
            case(Green, Green) => true
            case(Blue, Blue) => true
            case _ => false
        }
    }
    
    public override operator func !=(that: Colors): Bool {
        !(this == that)
    }
}



/*class DeclareKeyword ASC {
    constructor(param1:string, param2:{a:number,b:string}, param3:number, funcParam:()=>string, callback?:AsyncCallback<string>): void
    AS1(callback:AsyncCallback<void>): void
    }*/

public class ASC {
    
    protected ASC(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(param1:string, param2:{a:number,b:string}, param3:number, funcParam:()=>string, callback?:AsyncCallback<string>): void
	 */
    public init(param1: String, param2: JSValue/* FIXME: `:{a:number,b:string}` */, param3: Float64, funcParam: ()->String, callback!:?AsyncCallback<String> = None) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_asyncFunction", "ASC").asClass(__ctx)
            clazz.new(param1.toJSValue(__ctx), param2.toJSValue(__ctx), param3.toJSValue(__ctx), funcParam.toJSValue(__ctx), jsCallback).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_asyncFunction", "ASC").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    /**
	 * @brief AS1(callback:AsyncCallback<void>): void
	 */
    public static func AS1(callback: AsyncCallback<Unit>): Unit {
        let ctx = getMainContext()
        let jsCallback = asyncCallbackWrapper < Unit >(ctx, callback)
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_asyncFunction", "ASC"),  "AS1", { ctx =>[callback.toJSValue(ctx)] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): ASC {
        ASC(input.asObject())
    }
}

/*class DeclareKeyword ForParamToJs {
    constructor(param1:common.Context, param2:Colors, param3:BigInt, param4:Uint8Array, param5:number[], param6:Array<number>, callback:AsyncCallback<string>, paramEnum:Array<Colors>, param7?:Array<number>, param8?:Colors, param9?:Array<UInt8>): void
    }*/

public class ForParamToJs {
    
    protected ForParamToJs(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(param1:common.Context, param2:Colors, param3:BigInt, param4:Uint8Array, param5:number[], param6:Array<number>, callback:AsyncCallback<string>, paramEnum:Array<Colors>, param7?:Array<number>, param8?:Colors, param9?:Array<UInt8>): void
	 */
    public init(param1: JSValue/* FIXME: `common.Context` */, param2: Colors, param3: JSValue/* FIXME: `BigInt` */, param4: Array<UInt8>, param5: Array<Float64>, param6: Array<Float64>, callback: AsyncCallback<String>, paramEnum: Array<Colors>, param7!:?Array<Float64> = None, param8!:?Colors = None, param9!:?Array<UInt8> = None) {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_asyncFunction", "ForParamToJs").asClass(__ctx)
            clazz.new(getJSContext(mainThreadRuntime.getOrThrow(), param1), param2.get().toJSValue(__ctx), ctx.bigint(param3).toJSValue(ctx), toJSArray < UInt8 >(__ctx, param4, { __ctx, item => item.toJSValue(__ctx) }), toJSArray < Float64 >(__ctx, param5, { __ctx, item => item.toJSValue(__ctx) }), toJSArray < number >(__ctx, param6, { __ctx, item => item.toJSValue(ctx) }), jsCallback, toJSArray < Colors >(__ctx, paramEnum, { __ctx, item => item.get().toJSValue(ctx) }), toJSArray < number >(__ctx, param7, { __ctx, item => item.toJSValue(ctx) }),(param8?.get()).toJSValue(__ctx),(param9?.toJSValue(__ctx)) ?? __ctx.undefined().toJSValue()).asObject()
        }
    }
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_asyncFunction", "ForParamToJs").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): ForParamToJs {
        ForParamToJs(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief as1(callback:AsyncCallback<void>): void
	 */
public func as1(callback: AsyncCallback<Unit>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < Unit >(ctx, callback)
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as1", { ctx =>[callback.toJSValue(ctx)] })
}

/**
	 * @brief as2(callback:AsyncCallback<string>): void
	 */
public func as2(callback: AsyncCallback<String>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < String >(ctx, callback)
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as2", { ctx =>[callback.toJSValue(ctx)] })
}

/**
	 * @brief as3(callback:AsyncCallback<Colors>): void
	 */
public func as3(callback: AsyncCallback<Colors>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < Colors >(ctx, callback) {
        ctx, info => Colors.parse(Int32.fromJSValue(ctx, info))
    }
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as3", { ctx =>[callback.toJSValue(ctx)] })
}

/**
	 * @brief as4(callback:AsyncCallback<Array<number>>): void
	 */
public func as4(callback: AsyncCallback<Array<Float64>>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < Array<Float64> >(ctx, callback) {
        ctx, info =>
        let retArr = info.asArray()
        Array < Float64 >(retArr.size) {
            index => Float64.fromJSValue(ctx, retArr[index])
        }
    }
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as4", { ctx =>[callback.toJSValue(ctx)] })
}

/**
	 * @brief as5(callback:AsyncCallback<BigInt>): void
	 */
public func as5(callback: AsyncCallback<JSValue/* FIXME: `BigInt` */>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < JSValue/* FIXME: `BigInt` */ >(ctx, callback) {
        ctx, info => JSValue/* FIXME: `JSValue/*FIXME:`BigInt`*/` */.fromJSValue(ctx, info)
    }
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as5", { ctx =>[callback.toJSValue(ctx)] })
}

/**
	 * @brief as6(callback:AsyncCallback<Uint8Array>): void
	 */
public func as6(callback: AsyncCallback<Array<UInt8>>): Unit {
    let ctx = getMainContext()
    let jsCallback = asyncCallbackWrapper < Array<UInt8> >(ctx, callback)
    hmsGlobalApiCall < Unit >( "my_module_asyncFunction", "as6", { ctx =>[callback.toJSValue(ctx)] })
}


