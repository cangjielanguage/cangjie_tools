// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ohos

import ohos.ark_interop.JSModule
import ohos.ark_interop.JSContext
import ohos.ark_interop.JSCallInfo
import ohos.ark_interop.JSValue
import ohos.ark_interop.JSLambda
import ohos.ark_interop.JSFunction
import ohos.ark_interop.JSClass
import ohos.ark_interop.JSObject

func testCJ(runtime: JSContext, callInfo: JSCallInfo): JSValue {
    let result = "Hello ${callInfo[0].toString(runtime)}"
    runtime.string(result).toJSValue()
}

let EXPORT_MODULE = JSModule.registerModule {
    runtime, exports =>
        exports["testCJ"] = runtime.function(testCJ).toJSValue()
        exports["testClass"] = runtime.clazz(testCJ).toJSValue()
}
let fn: JSLambda = { context, callInfo =>
                return context.undefined().toJSValue()
            }

func fn_register(runtime: JSContext): JSFunction {
	runtime.function(fn)
}

func setDataId(runtime: JSContext, callInfo: JSCallInfo): JSValue {
	runtime.string("result").toJSValue()
}

func getDataId(runtime: JSContext, callInfo: JSCallInfo): JSValue {
	runtime.string("result").toJSValue()
}

func ccc(runtime: JSContext): JSClass {
	let ctor: JSLambda = { context, callInfo =>
        return callInfo.thisArg
    }
    let clazz = runtime.clazz(ctor)
    clazz.addMethod(runtime.string("setIdFunc"), runtime.function(setDataId))
    clazz.addMethod(runtime.string("getIdFunc"), runtime.function(getDataId))
    clazz.addMethod(runtime.string("lambdaFunc"), runtime.function{ runtime, callInfo => runtime.undefined().toJSValue()})
    clazz
}

func ddd(runtime: JSContext, exports: JSObject) {
	// func
    exports["testCJModuleFunc"] = runtime.function(testCJ).toJSValue()
    exports["testCJModuleFuncLambda"] = runtime.function{ context, callInfo => context.undefined().toJSValue()}.toJSValue()
    // class
    exports["testClassModuleFunc"] = runtime.clazz(testCJ).toJSValue() //.1
    exports["testClassModuleFuncLambda"] = runtime.clazz({context, callInfo => context.undefined().toJSValue()}).toJSValue() //.1
    let clazz = runtime.clazz(testCJ) //.2
    exports["testClassModuleFunc"] = clazz.toJSValue()
    clazz.addMethod(runtime.string("setId"), runtime.function(setDataId))
    clazz.addMethod(runtime.string("getId"), runtime.function(getDataId))
}


func xxxx() {
	// four
    JSModule.registerFunc("doSth", fn)
    JSModule.registerFunc("testCJ", testCJ)  // .1
    JSModule.registerFunc("doSthLambda") { context, callInfo =>  // .2
        return context.undefined().toJSValue()
    }

    // three
    JSModule.registerFunc("doSthThree", fn_register)  // .1
    JSModule.registerFunc("doSthThreeLambda") { context =>  // .2
        let fn: JSLambda = { context, callInfo =>
            return context.undefined().toJSValue()
        }
        return context.function(fn)
        
    }

    // tow
    JSModule.registerClass("SomeClass", ccc)  // .1
    JSModule.registerClass("SomeClassLambda") { context =>  //.2
        let ctor: JSLambda = { context, callInfo =>
            return callInfo.thisArg
        }
        let clazz = context.clazz(ctor)
        clazz.addMethod(context.string("setId"), context.function(setDataId))
        clazz.addMethod(context.string("getId"), context.function(getDataId))
        clazz.addMethod(context.string("lambda"), context.function{ context, callInfo => context.undefined().toJSValue()})
        clazz
    }

    // one
    JSModule.registerModule(ddd) //.1
    JSModule.registerModule {runtime, exports =>  //.2
        // func
        exports["testCJModule"] = runtime.function(testCJ).toJSValue()
        exports["testCJModuleLambda"] = runtime.function{ context, callInfo => context.undefined().toJSValue()}.toJSValue()
        exports["testCJModuleLambda"] = runtime.function{ context, callInfo => context.undefined().toJSValue()}.toJSValue()
        exports["testCJModuleLambda"] = runtime.function{context, callInfo => context.undefined().toJSValue()}.toJSValue()
        // class
        exports["testClassModule"] = runtime.clazz(testCJ).toJSValue() //.1
        exports["testClassModuleLambda"] = runtime.clazz({context, callInfo => context.undefined().toJSValue()}).toJSValue() //.1
        let clazz = runtime.clazz(testCJ) //.2
        exports["testClassModule"] = clazz.toJSValue()
        clazz.addMethod(runtime.string("setId"), runtime.function(setDataId))
        clazz.addMethod(runtime.string("getId"), runtime.function(getDataId))
    }
}


