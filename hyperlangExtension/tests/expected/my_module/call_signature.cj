// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package my_module

import ohos.ark_interop.*
import ohos.ark_interop_helper.*
import ohos.base.*
/*

public const when = 0/* FIXME: Initialization is required */
*/

/***********OBJECT***********/

/*interface DeclareKeyword when {
    afterReturn(value: any): any
    afterReturnNothing(): undefined
    afterAction(action: any): any
    afterThrow(e_msg: string): String
    (argMatchers?: any): when
    }*/
public open type when =(argMatchers!:?Any = None) -> Unit

/*interface ExportKeyword VerificationMode {
    times(count: Number): void
    never(): void
    once(): void
    atLeast(count: Number): void
    atMost(count: Number): void
    }*/

public open class VerificationMode {
    
    protected VerificationMode(public var arkts_object: JSObject) {}
    
    
    /**
	 * @brief times(count: Number): void
	 */
    public func times(count: JSValue/* FIXME: `Number` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "times", { ctx =>[count] })
    }
    /**
	 * @brief never(): void
	 */
    public func never(): Unit {
        jsObjApiCall < Unit >( arkts_object, "never", emptyArg)
    }
    /**
	 * @brief once(): void
	 */
    public func once(): Unit {
        jsObjApiCall < Unit >( arkts_object, "once", emptyArg)
    }
    /**
	 * @brief atLeast(count: Number): void
	 */
    public func atLeast(count: JSValue/* FIXME: `Number` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "atLeast", { ctx =>[count] })
    }
    /**
	 * @brief atMost(count: Number): void
	 */
    public func atMost(count: JSValue/* FIXME: `Number` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "atMost", { ctx =>[count] })
    }
    
    public open func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): VerificationMode {
        VerificationMode(input.asObject())
    }
}

/*class ExportKeyword ArgumentMatchers {
    StaticKeywordany: unsupported FirstToken;
    StaticKeywordanyString: unsupported FirstToken;
    StaticKeywordanyBoolean: unsupported FirstToken;
    StaticKeywordanyNumber: unsupported FirstToken;
    StaticKeywordanyObj: unsupported FirstToken;
    StaticKeywordanyFunction: unsupported FirstToken;
    matchRegexs(Regex: RegExp): void
    }*/

public class ArgumentMatchers {
    
    protected ArgumentMatchers(var arkts_object: JSObject) {}
    
    
    public static mut prop any: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["any"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["any"] = v
            }
        }
        
    }
    
    public static mut prop anyString: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyString"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyString"] = v
            }
        }
        
    }
    
    public static mut prop anyBoolean: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyBoolean"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyBoolean"] = v
            }
        }
        
    }
    
    public static mut prop anyNumber: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyNumber"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyNumber"] = v
            }
        }
        
    }
    
    public static mut prop anyObj: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyObj"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyObj"] = v
            }
        }
        
    }
    
    public static mut prop anyFunction: JSValue/* FIXME: `UnsupportedType[FirstToken]` */ {
        get() {
            checkThreadAndCall < JSValue/* FIXME: `UnsupportedType[FirstToken]` */ >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyFunction"]
            }
        }
        set(v) {
            checkThreadAndCall < Unit >(getMainContext()) {
                ctx: JSContext => getClassConstructorObj("my_module_callSignature", "ArgumentMatchers")["anyFunction"] = v
            }
        }
        
    }
    
    /**
	 * @brief matchRegexs(Regex: RegExp): void
	 */
    public static func matchRegexs(Regex: JSValue/* FIXME: `RegExp` */): Unit {
        jsObjApiCall < Unit >(getClassConstructorObj("my_module_callSignature", "ArgumentMatchers"),  "matchRegexs", { ctx =>[Regex] })
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): ArgumentMatchers {
        ArgumentMatchers(input.asObject())
    }
}

/*class ExportKeyword MockKit {
    constructor(): void
    mockFunc(obj: Object, func: Function): Function
    mockObject(obj: Object): Object
    verify(methodName: String, argsArray: Array<any>): VerificationMode
    ignoreMock(obj: Object, func: Function): void
    clear(obj: Object): void
    clearAll(): void
    }*/

public class MockKit {
    
    protected MockKit(var arkts_object: JSObject) {}
    /**
	 * @brief constructor(): void
	 */
    public init() {
        arkts_object = checkThreadAndCall < JSObject >(getMainContext()) {
            __ctx =>
            let clazz = getClassConstructorObj("cases_callSignature", "MockKit").asClass(__ctx)
            clazz.new().asObject()
        }
    }
    
    /**
	 * @brief mockFunc(obj: Object, func: Function): Function
	 */
    public func mockFunc(obj: JSValue/* FIXME: `Object` */, func: JSValue/* FIXME: `Function` */): JSValue/* FIXME: `Function` */ {
        jsObjApiCall < JSValue/* FIXME: `Function` */ >( arkts_object, "mockFunc", { ctx =>[obj, func] }) {
            ctx, info => info
        }
    }
    /**
	 * @brief mockObject(obj: Object): Object
	 */
    public func mockObject(obj: JSValue/* FIXME: `Object` */): JSValue/* FIXME: `Object` */ {
        jsObjApiCall < JSValue/* FIXME: `Object` */ >( arkts_object, "mockObject", { ctx =>[obj] }) {
            ctx, info => info
        }
    }
    /**
	 * @brief verify(methodName: String, argsArray: Array<any>): VerificationMode
	 */
    public func verify(methodName: String, argsArray: Array<Any>): VerificationMode {
        jsObjApiCall < VerificationMode >( arkts_object, "verify", { ctx =>[methodName.toJSValue(ctx), toJSArray < Any >(ctx, argsArray, { ctx: JSContext, val: Any => val.toJSValue(ctx) })] }) {
            ctx, info => VerificationMode.fromJSValue(ctx, info)
        }
    }
    /**
	 * @brief ignoreMock(obj: Object, func: Function): void
	 */
    public func ignoreMock(obj: JSValue/* FIXME: `Object` */, func: JSValue/* FIXME: `Function` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "ignoreMock", { ctx =>[obj, func] })
    }
    /**
	 * @brief clear(obj: Object): void
	 */
    public func clear(obj: JSValue/* FIXME: `Object` */): Unit {
        jsObjApiCall < Unit >( arkts_object, "clear", { ctx =>[obj] })
    }
    /**
	 * @brief clearAll(): void
	 */
    public func clearAll(): Unit {
        jsObjApiCall < Unit >( arkts_object, "clearAll", emptyArg)
    }
    
    func toJSValue(context: JSContext): JSValue {
        arkts_object.toJSValue()
    }
    
    static func fromJSValue(context: JSContext, input: JSValue): MockKit {
        MockKit(input.asObject())
    }
}

/***********METHOD***********/
/**
	 * @brief MockSetup(target: Object, propertyName: string | Symbol, descriptor: TypedPropertyDescriptor<() => void>): void
	 */
public func MockSetup(target: JSValue/* FIXME: `Object` */, propertyName: String, descriptor: TypedPropertyDescriptor<() -> Unit>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_callSignature", "MockSetup", { ctx =>[target, propertyName.toJSValue(ctx), descriptor.toJSValue(ctx)] })
}

/**
	 * @brief MockSetup(target: Object, propertyName: string | Symbol, descriptor: TypedPropertyDescriptor<() => void>): void
	 */
public func MockSetup(target: JSValue/* FIXME: `Object` */, propertyName: JSValue/* FIXME: `Symbol` */, descriptor: TypedPropertyDescriptor<() -> Unit>): Unit {
    hmsGlobalApiCall < Unit >( "my_module_callSignature", "MockSetup", { ctx =>[target, propertyName, descriptor.toJSValue(ctx)] })
}


