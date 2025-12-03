// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.time.DateTime

public func createArkAPICallAsync(gconfig: GlobalConfig): String {
    let moduleName = gconfig.cjModuleName

    let content = """
// Copyright (c) Huawei Technologies Co., Ltd. ${DateTime.nowUTC().year}. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package ${moduleName}

import std.collection.{concurrent.ConcurrentHashMap, HashMap}
import std.collection.{all, any}
import std.sync.ReentrantMutex
import ohos.ark_interop.*
import std.convert.*
import std.process.*
import std.time.*
import ohos.base.UIThread

foreign func gettid(): Int64

const JS_INTERNAL_ERROR = -1i32
protected let emptyArg = {ctx: JSContext => Array<JSValue>()}
private let MAIN_CONTEXT_MODULE_MAP = ConcurrentHashMap<String, JSObject>()
protected var mainThreadRuntime = None<JSRuntime>
protected var mainThreadContext = None<JSContext>
private var lock = ReentrantMutex()
private let mainThreadId = Process.current.pid
// only run in hybrid mode
private let REG_CTX = JSModule.registerModule {
    ctx, _ => mainThreadContext = ctx
}

protected func getMainContext(): JSContext {
    // hybrid mode
    if (let Some(v) <- mainThreadContext) {
        return v
    }
    // pure cj, create js runtime in main thread
    // user may call this func in spawn

    func initJs(): JSContext {
        synchronized(lock) {
            if (let Some(v) <- mainThreadContext) {
                v
            } else {
                let rt = JSRuntime()
                mainThreadRuntime = rt
                mainThreadContext = rt.mainContext
                rt.mainContext
            }
        }
    }

    let curTid = unsafe { gettid() }
    if (curTid == mainThreadId) {
        initJs()
    } else {
        let f = spawn (UIThread) {
            initJs()
        }
        f.get()
    }
}

protected func checkThreadAndCall<T>(ctx: JSContext, f: (JSContext) -> T): T {
    if (ctx.isInBindThread()) {
        f(ctx)
    } else {
        let fut = spawn (UIThread) {
            f(ctx)
        }
        fut.get()
    }
}

func handleAsyncError(ctx: JSContext, err: JSValue): BusinessException {
    if (err.isObject()) {
        let errObj = err.asObject()
        let code = if (errObj["code"].isNumber()) {
            Int32.fromJSValue(ctx, errObj.getProperty("code"))
        } else if (errObj["code"].isString()) {
            Int32.tryParse(String.fromJSValue(ctx, errObj["code"])) ?? JS_INTERNAL_ERROR
        } else {
            JS_INTERNAL_ERROR
        }
        let msg = if (errObj["message"].isString()) {
            String.fromJSValue(ctx, errObj["message"])
        } else {
            "unknown exception occured"
        }
        BusinessException(code, msg)
    } else if (err.isNumber()) {
        let code = Int32.fromJSValue(ctx, err)
        BusinessException(code, "unknown exception occured")
    } else {
        BusinessException(JS_INTERNAL_ERROR, "unknown exception occured")
    }
}

protected func asyncCallbackWrapper<T>(ctx: JSContext, callback: AsyncCallback<T>, paramParser: (JSContext, JSValue) -> T): JSValue {
    func call(ctx: JSContext): JSValue {
        let jscall = ctx.function {
            ctx, info =>
            try {
                var err = None<BusinessException>
                var data = None<T>
                if (!info[0].isNull() && !info[0].isUndefined()) {
                    err = handleAsyncError(ctx, info[0])
                }
                let infoCount = info.count
                //arkts 异常情况可能只返回异常，没有数值，info count此时为1
                if (infoCount == 2 && !info[1].isNull() && !info[1].isUndefined()) {
                    data = paramParser(ctx, info[1])
                }
                callback(err, data)
            } catch (e: Exception) {
                Hilog.error(1, "hle", e.toString())
                throw e
            }
            ctx.undefined().toJSValue()
        }
        jscall.toJSValue()
    }
    checkThreadAndCall<JSValue>(ctx, call)
}

protected func asyncCallbackFromWrapper<T>(ctx: JSContext, value: JSValue): AsyncCallback<T> where T <: JSInteropType<T> {
    let callback: AsyncCallback<T> = {
        e: ?BusinessException, item: ?T =>
        let params: Array<JSValue> = [e?.toJSValue(ctx) ?? ctx.undefined().toJSValue(),
            item?.toJSValue(ctx) ?? ctx.undefined().toJSValue()]
        value.asFunction().call(params)
    }
    return callback
}

protected func asyncCallbackWrapper2Arg<T, P>(ctx: JSContext, callback: (?BusinessException, ?T, ?P) -> Unit,
    paramParser1: (JSContext, JSValue) -> ?T, paramParser2: (JSContext, JSValue) -> ?P): JSValue {
    func call(ctx: JSContext): JSValue {
        let jscall = ctx.function {
            ctx, info =>
            try {
                var err = None<BusinessException>
                var data1 = None<T>
                var data2 = None<P>
                if (!info[0].isNull() && !info[0].isUndefined()) {
                    err = handleAsyncError(ctx, info[0])
                } else {
                    data1 = paramParser1(ctx, info[1])
                    data2 = paramParser2(ctx, info[2])
                }
                callback(err, data1, data2)
            } catch (e: Exception) {
                Hilog.error(1, "hle", e.toString())
                throw e
            }
            ctx.undefined().toJSValue()
        }
        jscall.toJSValue()
    }
    checkThreadAndCall<JSValue>(ctx, call)
}

protected func asyncCallbackWrapper<T>(ctx: JSContext, callback: AsyncCallback<T>): JSValue where T <: JSInteropType<T> {
    asyncCallbackWrapper<T>(ctx, callback) {ctx, info => T.fromJSValue(ctx, info)}
}

protected func asyncCallbackWrapperEx<T>(ctx: JSContext, callback: AsyncCallback<T>,
    paramParser: (JSContext, JSValue) -> T): JSValue {
    func call(ctx: JSContext): JSValue {
        let jscall = ctx.function {
            ctx, info =>
            try {
                var err = None<BusinessException>
                var data = None<T>
                if (info[1].isNull() || info[1].isUndefined()) {
                    err = handleAsyncError(ctx, info[0])
                } else {
                    data = paramParser(ctx, info[1])
                }
                callback(err, data)
            } catch (e: Exception) {
                Hilog.error(1, "hle", e.toString())
                throw e
            }
            ctx.undefined().toJSValue()
        }
        jscall.toJSValue()
    }
    checkThreadAndCall<JSValue>(ctx, call)
}

protected func getJSModule(ctx: JSContext, name: String, prefix: ?String): JSObject {
    match (MAIN_CONTEXT_MODULE_MAP.get(name)) {
        case Some(v) => v
        case None =>
            let module =  ctx.global[name].asObject(ctx)
            MAIN_CONTEXT_MODULE_MAP.add(name, module)
            module
    }
}

protected func toJSFunction(fun: () -> Unit): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info =>
            fun()
            ctx.undefined().toJSValue()
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func toJSFunction<T>(fun: (T) -> Unit): JSValue where T <: JSInteropType<T> {
    toJSFunction(fun, {ctx, info => T.fromJSValue(ctx, info)})
}

protected func toJSFunction<T>(fun: (T) -> Unit, paramParser: (JSContext, JSValue) -> T): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info =>
            fun(paramParser(ctx, info[0]))
            ctx.undefined().toJSValue()
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func toJSFunction<T, P>(fun: (T, P) -> Unit, paramParser1: (JSContext, JSValue) -> T,
    paramParser2: (JSContext, JSValue) -> P): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info =>
            fun(paramParser1(ctx, info[0]), paramParser2(ctx, info[1]))
            ctx.undefined().toJSValue()
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func toJSFunction<P1, P2, P3>(fun: (P1, P2, P3) -> Unit, paramParser1: (JSContext, JSValue) -> P1,
    paramParser2: (JSContext, JSValue) -> P2, paramParser3: (JSContext, JSValue) -> P3): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info =>
            fun(paramParser1(ctx, info[0]), paramParser2(ctx, info[1]), paramParser3(ctx, info[2]))
            ctx.undefined().toJSValue()
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func toJSFunctionWithReturn<P1, P2, R>(fun: (P1, P2) -> R, paramParser1: (JSContext, JSValue) -> P1,
    paramParser2: (JSContext, JSValue) -> P2, retParser: (JSContext, R) -> JSValue): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info => retParser(ctx, fun(paramParser1(ctx, info[0]), paramParser2(ctx, info[1])))
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func toJSFunctionWithReturn<T, P, J, K>(fun: (T, P, J) -> K, param1Parser: (JSContext, JSValue) -> T,
    param2Parser: (JSContext, JSValue) -> P, param3Parser: (JSContext, JSValue) -> J,
    retParser: (JSContext, K) -> JSValue): JSValue {
    func call(ctx: JSContext): JSValue {
        let jsFun = ctx.function {
            ctx, info => retParser(ctx,
                fun(param1Parser(ctx, info[0]), param2Parser(ctx, info[1]), param3Parser(ctx, info[2])))
        }
        jsFun.toJSValue()
    }
    checkThreadAndCall<JSValue>(getMainContext(), call)
}

protected func fromJSFunction<T>(ctx: JSContext, fun: JSValue): (T) -> Unit where T <: JSInteropType<T> {
    fromJSFunction(ctx, fun, {ctx, val => val.toJSValue(ctx)})
}

protected func fromJSFunction<T>(ctx: JSContext, fun: JSValue, paramParser: (JSContext, T) -> JSValue): (T) -> Unit {
    {
        val: T => checkThreadAndCall<Unit>(ctx, {ctx => fun.asFunction().call([paramParser(ctx, val)])})
    }
}

protected func fromJSFunctionWithReturn<T, P, K>(ctx: JSContext, fun: JSValue,
    param1Parser: (JSContext, T) -> JSValue, param2Parser: (JSContext, P) -> JSValue,
    retParser: (JSContext, JSValue) -> K): (T, P) -> K {
    {
        val1: T, val2: P => checkThreadAndCall<K>(
            ctx,
            {
                ctx => retParser(ctx,
                    fun.asFunction().call([param1Parser(ctx, val1), param2Parser(ctx, val2)]))
            }
        )
    }
}

protected func fromJSFunctionWithReturn<T, P, J, K>(ctx: JSContext, fun: JSValue,
    param1Parser: (JSContext, T) -> JSValue, param2Parser: (JSContext, P) -> JSValue,
    param3Parser: (JSContext, J) -> JSValue, retParser: (JSContext, JSValue) -> K): (T, P, J) -> K {
    {
        val1: T, val2: P, val3: J => checkThreadAndCall<K>(
            ctx,
            {
                ctx => retParser(ctx,
                    fun.asFunction().call([param1Parser(ctx, val1), param2Parser(ctx, val2), param3Parser(ctx, val3)]))
            }
        )
    }
}

protected func fromJSFunction<T, P>(ctx: JSContext, fun: JSValue, paramParser1: (JSContext, T) -> JSValue,
    paramParser2: (JSContext, P) -> JSValue): (T, P) -> Unit {
    {
        val1: T, val2: P => checkThreadAndCall<Unit>(ctx,
            {ctx => fun.asFunction().call([paramParser1(ctx, val1), paramParser2(ctx, val2)])})
    }
}

protected func fromJSFunction<P1, P2, P3>(ctx: JSContext, fun: JSValue, paramParser1: (JSContext, P1) -> JSValue,
    paramParser2: (JSContext, P2) -> JSValue, paramParser3: (JSContext, P3) -> JSValue): (P1, P2, P3) -> Unit {
    {
        val1: P1, val2: P2, val3: P3 => checkThreadAndCall<Unit>(ctx,
            {ctx => fun.asFunction().call([paramParser1(ctx, val1), paramParser2(ctx, val2), paramParser3(ctx, val3)])})
    }
}

protected func jsGlobalApiCall<T>(moduleName: String, modulePrefix: ?String, funcName: String,
    args: (JSContext) -> Array<JSValue>, onResolve: (JSContext, JSValue) -> T): T {
    func call(context: JSContext): T {
        try {
            let jsModule = getJSModule(context, moduleName, modulePrefix)
            let jsRet = jsModule.callMethod(funcName, args(context))
            onResolve(context, jsRet)
        } catch (e: JSCodeError) {
            throw BusinessException(e.code.getOrDefault({ => 0}), e.jsError)
        }
    }
    checkThreadAndCall<T>(getMainContext(), call)
}

protected func jsObjApiCall<T>(obj: JSObject, funcName: String, args: (JSContext) -> Array<JSValue>,
    onResolve: (JSContext, JSValue) -> T): T {
    func call(context: JSContext): T {
        try {
            let jsRet = obj.callMethod(funcName, args(context))
            onResolve(context, jsRet)
        } catch (e: JSCodeError) {
            throw BusinessException(e.code.getOrDefault({ => 0}), e.jsError)
        }
    }
    checkThreadAndCall<T>(getMainContext(), call)
}

protected func hmsGlobalApiCall<T>(moduleName: String, funcName: String, args: (JSContext) -> Array<JSValue>,
    onResolve: (JSContext, JSValue) -> T): T {
    jsGlobalApiCall<T>(moduleName, "hms", funcName, args, onResolve)
}

protected func ohosGlobalApiCall<T>(moduleName: String, funcName: String, args: (JSContext) -> Array<JSValue>,
    onResolve: (JSContext, JSValue) -> T): T {
    jsGlobalApiCall<T>(moduleName, None<String>, funcName, args, onResolve)
}

protected func jsObjApiCall<T>(obj: JSObject, funcName: String, args: (JSContext) -> Array<JSValue>): T where T <: JSInteropType<T> {
    jsObjApiCall<T>(obj, funcName, args) {ctx, info => T.fromJSValue(ctx, info)}
}

protected func hmsGlobalApiCall<T>(moduleName: String, funcName: String, args: (JSContext) -> Array<JSValue>): T where T <: JSInteropType<T> {
    hmsGlobalApiCall<T>(moduleName, funcName, args) {ctx, info => T.fromJSValue(ctx, info)}
}

protected func ohosGlobalApiCall<T>(moduleName: String, funcName: String, args: (JSContext) -> Array<JSValue>): T where T <: JSInteropType<T> {
    ohosGlobalApiCall<T>(moduleName, funcName, args) {ctx, info => T.fromJSValue(ctx, info)}
}

// prefix = "None"/"hms"
protected func getClassConstructorObj(moduleName: String, className: String, prefix!: ?String = None): JSObject {
    checkThreadAndCall<JSObject>(getMainContext()) {
        ctx =>
        let module = getJSModule(ctx, moduleName, prefix)
        module[className].asObject()
    }
}

protected func jsGlobalApiCallPromise<T>(moduleName: String, modulePrefix: ?String, funcName: String,
    callback: AsyncCallback<T>, args: (JSContext) -> Array<JSValue>, onResolve: (JSContext, JSValue) -> T): Unit {
    func call(context: JSContext): Unit {
        try {
            let jsRet = getJSModule(context, moduleName, modulePrefix).callMethod(funcName, args(context))
            let promise = jsRet.asPromise(context)
            let onResolved = context.function(
                {
                    ctx: JSContext, callInfo: JSCallInfo =>
                    try {
                        let result = onResolve(ctx, callInfo[0])
                        callback(None, result)
                    } catch (e: BusinessException) {
                        callback(e, None)
                    } catch (e: Exception) {
                        callback(BusinessException(JS_INTERNAL_ERROR, e.message), None)
                    }
                    ctx.null().toJSValue()
                }
            )
            let onRejected = context.function(
                {
                    ctx: JSContext, callInfo: JSCallInfo =>
                    callback(handleAsyncError(ctx, callInfo[0]), None)
                    ctx.null().toJSValue()
                }
            )
            promise.then(onResolved, onRejected: onRejected)
        } catch (e: JSCodeError) {
            throw BusinessException(e.code.getOrDefault({ => 0}), e.jsError)
        }
    }
    checkThreadAndCall<Unit>(getMainContext(), call)
}

protected func jsGlobalApiCallPromise<T>(moduleName: String, modulePrefix: ?String, funcName: String,
    callback: AsyncCallback<T>, args: (JSContext) -> Array<JSValue>): Unit where T <: JSInteropType<T> {
    jsGlobalApiCallPromise<T>(moduleName, modulePrefix, funcName, callback, args) {
        ctx, value => T.fromJSValue(ctx, value)
    }
}

protected func hmsGlobalApiCallPromise<T>(moduleName: String, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>, onResolve: (JSContext, JSValue) -> T): Unit {
    jsGlobalApiCallPromise<T>(moduleName, "hms", funcName, callback, args, onResolve)
}

protected func ohosGlobalApiCallPromise<T>(moduleName: String, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>, onResolve: (JSContext, JSValue) -> T): Unit {
    jsGlobalApiCallPromise<T>(moduleName, None, funcName, callback, args, onResolve)
}

protected func hmsGlobalApiCallPromise<T>(moduleName: String, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>): Unit where T <: JSInteropType<T> {
    jsGlobalApiCallPromise<T>(moduleName, "hms", funcName, callback, args)
}

protected func ohosGlobalApiCallPromise<T>(moduleName: String, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>): Unit where T <: JSInteropType<T> {
    jsGlobalApiCallPromise<T>(moduleName, None, funcName, callback, args)
}

protected func jsObjApiCallPromise<T>(obj: JSObject, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>, onResolve: (JSContext, JSValue) -> T): Unit {
    func call(context: JSContext): Unit {
        try {
            let jsRet = obj.callMethod(funcName, args(context))
            let promise = jsRet.asPromise(context)
            let onResolved = context.function(
                {
                    ctx: JSContext, callInfo: JSCallInfo =>
                    try {
                        let result = onResolve(ctx, callInfo[0])
                        callback(None, result)
                    } catch (e: BusinessException) {
                        callback(e, None)
                    } catch (e: Exception) {
                        callback(BusinessException(JS_INTERNAL_ERROR, e.message), None)
                    }
                    ctx.null().toJSValue()
                }
            )
            let onRejected = context.function(
                {
                    ctx: JSContext, callInfo: JSCallInfo =>
                    callback(handleAsyncError(ctx, callInfo[0]), None)
                    ctx.null().toJSValue()
                }
            )
            promise.then(onResolved, onRejected: onRejected)
        } catch (e: JSCodeError) {
            throw BusinessException(e.code.getOrDefault({ => 0}), e.jsError)
        }
    }
    checkThreadAndCall<Unit>(getMainContext(), call)
}

protected func jsObjApiCallPromise<T>(obj: JSObject, funcName: String, callback: AsyncCallback<T>,
    args: (JSContext) -> Array<JSValue>): Unit where T <: JSInteropType<T> {
    jsObjApiCallPromise(obj, funcName, callback, args) {ctx, value => T.fromJSValue(ctx, value)}
}

protected func toJSArray<T>(ctx: JSContext, cjArr: ?Array<T>): JSValue where T <: JSInteropType<T> {
    toJSArray<T>(ctx, cjArr) {ctx, item => item.toJSValue(ctx)}
}

protected func toJSArray<T>(ctx: JSContext, cjArr: ?Array<T>, constructor: (JSContext, T) -> JSValue): JSValue {
    if (let Some(arr) <- cjArr) {
        let jsArr = Array<JSValue>(arr.size) {
            i => constructor(ctx, arr[i])
        }
        ctx.array(jsArr).toJSValue()
    } else {
        ctx.undefined().toJSValue()
    }
}

protected func fromJSArray<T>(ctx: JSContext, input: JSValue): Array<T> where T <: JSInteropType<T> {
    fromJSArray<T>(ctx, input, T.fromJSValue)
}

protected func fromJSArray<T>(ctx: JSContext, input: JSValue, constructor: (JSContext, JSValue) -> T): Array<T> {
    if (input.isUndefined()) {
        return []
    }
    let jsArr = input.asArray(ctx)
    return Array<T>(jsArr.size) {
        index => constructor(ctx, jsArr[index])
    }
}

protected func fromJSArrayOption<T>(ctx: JSContext, input: JSValue): Option<Array<T>> where T <: JSInteropType<T> {
    if (input.isUndefined()) {
        None<Array<T>>
    } else {
        Some(fromJSArray<T>(ctx, input))
    }
}

protected func fromJSArrayOption<T>(ctx: JSContext, input: JSValue, constructor: (JSContext, JSValue) -> T): Option<Array<T>> {
    if (input.isUndefined()) {
        None<Array<T>>
    } else {
        Some(fromJSArray(ctx, input, constructor))
    }
}

public interface FromJSValue<T> {
    static func fromJSValue(context: JSContext, value: JSValue): T
}

public interface ToJSValue {
    func toJSValue(ctx: JSContext): JSValue
}

extend BusinessException <: FromJSValue<BusinessException> & ToJSValue {
    public static func fromJSValue(context: JSContext, err: JSValue): BusinessException {
        handleAsyncError(context, err)
    }

    public func toJSValue(context: JSContext): JSValue {
        let obj = context.object()
        obj["code"] = this.code.toJSValue(context)
        obj["data"] = this.message.toJSValue(context)
        obj.toJSValue()
    }
}

extend DateTime <: FromJSValue<DateTime> & ToJSValue {
    public func toJSValue(context: JSContext): JSValue {
        let dateClazz = context.global["Date"].asClass()
        dateClazz.new(this.toUnixTimeStamp().toMilliseconds().toJSValue(context))
    }

    public static func fromJSValue(context: JSContext, input: JSValue): DateTime {
        let date = input.asObject()
        let ms = date.callMethod("getTime")
        DateTime.fromUnixTimeStamp(Duration.millisecond * Int64.fromJSValue(context, ms))
    }
}

protected func record2Hashmap<V>(context: JSContext, input: JSValue, valueConstructor: (JSContext, JSValue) -> V): ?HashMap<String, V> {
    if (input.isUndefined()) {
        None
    } else {
        let record = input.asObject()
        let hashmap = HashMap<String, V>()

        for(key in record.keys()) {
            hashmap.add(key, valueConstructor(context, record[key]))
        }
        hashmap
    }
}

protected func hashmap2Record<V>(context: JSContext, hashmap: ?HashMap<String, V>, valueConstructor: (JSContext, V) -> JSValue): JSValue {
    if (let Some(map) <- hashmap) {
        let obj = context.object()

        for((k, v) in map) {
            obj.setProperty(k, valueConstructor(context, v))
        }

        obj.toJSValue()
    } else {
        context.undefined().toJSValue()
    }
}

// deprecated, use context.arrayBuffer(cjArray).toUInt8ArrayJSValue() to directly convert Cangjie array to typed array
protected func typedArray2JSArray(context: JSContext, input: JSValue): JSValue {
    context.global["Array"].asObject().callMethod("from", [input])
}

// deprecated, use input.asArrayBuffer(context).toUInt8Array() to directly convert JSValue to Cangjie array
protected func jsArray2TypedArray(context: JSContext, input: JSValue, arrayClass: String): JSValue {
    context.global[arrayClass].asClass().new(input)
}

protected func instanceof(value: JSValue, className: String): Bool {
    let ctx = getMainContext()
    let prototype = ctx.global["Object"].asObject().callMethod("getPrototypeOf", [value]).asObject()
    let constructor = prototype["constructor"].asObject()
    return constructor["name"].toString() == className
}

protected func instanceof(value: JSValue, moduleName: String, prefix: ?String, className: String): Bool {
    func innerInstanceof(ctx: JSContext): Bool {
        let clazz = getJSModule(ctx, moduleName, prefix)[ctx.string(className)].asClass(ctx)
        value.asObject().instanceOf(clazz)
    }
    checkThreadAndCall<Bool>(getMainContext(), innerInstanceof)
}

// check if the input is an object and has all the properties
protected func propertyCheck(input: JSValue, properties: Array<String>): Bool {
    try {
        let obj = input.asObject()
        properties |> all { propName =>
            obj.hasProperty(propName)
        }
    } catch (_) {
        false
    }
}

// check if the input is an object and has ANY of the properties
protected func propertyCheckAny(input: JSValue, properties: Array<String>): Bool {
    try {
        let obj = input.asObject()
        properties |> any { propName =>
            obj.hasProperty(propName)
        }
    } catch (_) {
        false
    }
}
"""
    return content
}
