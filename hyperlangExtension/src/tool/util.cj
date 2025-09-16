// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.fs.*
import std.ast.*
import std.collection.*
import std.regex.*

public func addComment(msg: String, reason!: ?String = None): Tokens {
    if (let Some(r) <- reason) {
        Tokens([Token(TokenKind.COMMENT, "// ${r}"), nl, Token(TokenKind.COMMENT, "/*${msg}*/"), nl])
    } else {
        Tokens([Token(TokenKind.COMMENT, "/*${msg}*/"), nl])
    }
}

public func dedupFuncName(name: String): String {
    match (name) {
        case "init" | "getObject" | "generateJSO" => "_" + name //以防函数名为仓颉关键字或者重名，此处可扩展
        case _ => name
    }
}

public func dedupVarName(name: String): String {
    match (name) {
        case "type" | "extra" => "_" + name.replace("?", "") //以防函数名为仓颉关键字或者重名，此处可扩展
        case _ => name.replace("?", "")
    }
}

public func checkCangjieKeyword(name: String): String {
    if (cangjieKeyword.contains(name)) {
        "`${name}`"
    } else {
        name
    }
}

/**
 * @return Array & inner type
 */
public func isArray(typeStr: String): Bool {
    // Array<Byte> is JSInteropType, can use fromJSValue/toJSValue
    if (typeStr == "Array<Byte>" || typeStr == "Array<UInt8>") {
        return false
    }
    typeStr.startsWith("Array<") && typeStr.endsWith(">")
}

/**
 * @return BigInt
 */
public func isBigInt(typeStr: String): Bool {
    typeStr == "BigInt"
}

/**
 * @return Uint8Array
 */
public func isUint8Array(typeStr: String): Bool {
    typeStr == "Uint8Array"
}

/**
 * @return number[]
 */
public func isNumberArray(typeStr: String): Bool {
    typeStr == "number[]"
}

/**
 * @return ArrayType
 */
public func isArrayType(typeStr: String): Bool {
    typeStr.startsWith("Array<") && typeStr.endsWith(">")
}

public func isCallback(typeStr: String): Bool {
    Regex(#"(^Callback<.*>$)|(\(.*\)\=>.*)"#).matches(typeStr.replace(" ", ""))
}

public func getArrayInnerType(arrType: String): String {
    var innerType = arrType[6..(arrType.size - 1)]
    if (innerType == "string") {
        return "String"
    } else {
        return innerType
    }
}

public func mergeCommentToken(arkComment: String, brief: String): Token {
    let sinceIdx = arkComment.indexOf("@since") ?? arkComment.size
    let subArkComment = arkComment[sinceIdx..]
    let nextCRLF = subArkComment.indexOf("\n") ?? 0

    var comment = "/**\n\t * " + "@brief ${brief}" + "\n\t */"
    if (arkComment.contains("@since")) {
        comment = arkComment[..sinceIdx] + "@brief ${brief}" + subArkComment[nextCRLF..]
    } else {
        if (arkComment.size != 0) {
            comment = comment + "\n\t" + arkComment
        }
    }
    comment = comment
        .replace("{ boolean }", "{ Bool }")
        .replace("{ string }", "{ String }")
        .replace("{ BusinessError }", "{ BusinessException }")
    Token(TokenKind.COMMENT, comment)
}

public func isAsyncCallback(typeStr: String): Bool {
    typeStr.startsWith("AsyncCallback<") && typeStr.endsWith(">")
}

public func transAsyncCallback(typeStr: String): String {
    "AsyncCallback<${typeStr}>"
}

public func funcRetPromiseOnly(methodType: MethodType): Bool {
    let parentMethods = methodType.namespace?.methods ?? methodType.obj?.methods ?? throw IllegalArgumentException(
        "method parent not found")
    for (method in parentMethods) {
        if (refEq(method, methodType)) {
            continue
        }
        if (method.name == methodType.name) {
            if (method.returns.contains(promise)) {
                continue
            }
            let paramSize = methodType.params.size
            if (method.params.size < paramSize) {
                continue
            }
            if (paramSize == 1) {
                continue
            }
            if (methodType.params == method.params[..paramSize]) {
                return false
            }
        }
    }
    return true
}

public func checkAPIAndCommentOut(token: Tokens, comment: String): Tokens {
    if (comment.contains("@systemapi")) {
        return addComment(token.toString(), reason: "this is systemapi")
    }
    if (comment.contains("@deprecated")) {
        return addComment(token.toString(), reason: "this api is deprecated")
    }
    token
}

public func removeTsSuffix(name: String): String {
    if (name.endsWith(".d.ts")) {
        name[..name.size - 5]
    } else if (name.endsWith(".d.ets")) {
        name[..name.size - 6]
    } else {
        name
    }
}

public func camelToUnderscore(name: String): String {
    var result = StringBuilder()
    var index = 0
    for (c in name) {
        if (c.isAsciiUpperCase()) {
            // Avoid appending extra '_', eg: Index.d.ts or X_Index.d.ts
            if (index != 0 && Rune(name[index - 1]) != r"_") {
                result.append("_")
            }
            result.appendFromUtf8(c.toAsciiLowerCase())
        } else {
            result.appendFromUtf8(c)
        }
        index += 1
    }
    result.toString()
}

public func max<T>(a: T, b: T, others: Array<T>): T where T <: Comparable<T> {
    var max = if (a > b) {
        a
    } else {
        b
    }
    if (others.size == 0) {
        return max
    }
    for (item in others) {
        if (item > max) {
            max = item
        }
    }
    return max
}

public func hasEventAPI(methods: ArrayList<MethodType>): Bool {
    for (m in methods) {
        if (isEventAPI(m)) {
            return true
        }
    }
    false
}

public func isEventAPI(method: MethodType): Bool {
    return (method.name == "on" || method.name == "off") && method.params.size >= 2;
}

public func declareCallbackManager(): Tokens {
    quote(let callbackManager = CallbackManager<String, JSValue>()) + nl + nl
}

public func getParentName(base: String): String {
    if (ViewBase.contains(base) || base == "Canvas") {
        return "ViewBase"
    } else if (ContainerBase.contains(base)) {
        return "ContainerBase"
    } else if (ShapeComponent.contains(base)) {
        return "ShapeComponent"
    }
    return ""
}

public func getCmToken(str: String): Tokens {
    return Token(TokenKind.COMMENT, str) + nl
}

public func getTokenWithoutNL(str: String): Tokens {
    let res = Tokens()
    return res.append(Token(TokenKind.COMMENT, str))
}

public func getParamsFromUnionType(arkMethod: MethodType): ArrayList<ArrayList<String>> {
    var resParams: ArrayList<ArrayList<String>> = ArrayList<ArrayList<String>>()
    for (param in arkMethod.params) {
        let names = param.replace(" ", "").split(":")
        if (names.size != 2) {
            continue
        }
        let unionNames = if (names[1].startsWith("{") || names[1].contains("Record")) {
            [names[1]]
        } else {
            names[1].split("|")
        }
        if (resParams.size == 0) {
            for (unionName in unionNames) {
                resParams.add(ArrayList<String>([names[0] + ":" + unionName]))
            }
            continue
        }
        resParams = resParams |>
            flatMap {
            it: ArrayList<String> => unionNames |>
                 map {
                name: String =>
                let newArray = it.clone();
                newArray.add(names[0] + ":" + name);
                ArrayList<String>(newArray)
            }
        } |> collectArrayList<ArrayList<String>>
    }
    if (resParams.size == 0) {
        resParams.add(arkMethod.params)
    }
    return resParams
}

public func splitUnionParams(params: ArrayList<TsParameter>): Array<ArrayList<TsParameter>> {
    var result: Array<ArrayList<TsParameter>> = [params]
    for (i in 0..params.size) {
        let p = params[i]
        if (let TsUnion(types) <- p.ty) {
            // Do not split union type which pattern is "T|null" or "T|undefined"
            let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
            for (e in types) {
                match (e) {
                    case TsNull | TsUndefined => ()
                    case _ => nonNullTypes.add(e)
                }
            }
            if (nonNullTypes.size == 1) {
                continue
            }
            result = result |>
                flatMap {
                r => types.map {
                    t => ArrayList(r.size, {
                        j => if (j == i) {
                            TsParameter(p.name, t, p.optional)
                        } else {
                            r[j]
                        }
                    })
                }
            } |> collectArray
        }
    }
    result
}

public func createUnionType(typeName: String, tokens: Tokens, unionTypeSet: ArrayList<String>): String {
    if (typeName.contains("|")) {
        let unionTypeName = "Union_" + (typeName.replace(" ", "").split("|") |> map {it => TypeMap.typeMap(it)} |>
            collectString<String>(delimiter: "_"))
        if (!unionTypeSet.contains(unionTypeName)) {
            tokens.append(getCmToken("public interface ${unionTypeName} {}"))
            let subTypes = typeName.replace(" ", "").split("|").map {it => TypeMap.typeMap(it)}
            for (subType in subTypes) {
                tokens.append(getCmToken("extend ${subType} <: ${unionTypeName} {}"))
            }
            // generate union2JSValue
            tokens.append(getCmToken("func union2JSValue(context: JSContext, input: ${unionTypeName}): JSValue {"))
            tokens.append(getCmToken("    match (input) {"))
            subTypes
                .iterator()
                .forEach {it: String => tokens.append(getCmToken("        case v: ${it} => v.toJSValue(context)"))}
            tokens.append(getCmToken("        case _ => throw IllegalArgumentException(\"Not support Type\")"))
            tokens.append(getCmToken("    }"))
            tokens.append(getCmToken("}")).append(nl)
            // generate FromJSValue
            tokens.append(
                getCmToken("func ${unionTypeName}FromJSValue(objType: String, input: JSValue): ${unionTypeName} {"))
            tokens.append(getCmToken("    match (objType) {"))
            subTypes
                .iterator()
                .forEach {
                    it: String => tokens.append(
                        getCmToken("        case \"${it}\" => ${it}.fromJSValue(getCtx(), input)"))
                }
            tokens.append(getCmToken("        case _ => throw IllegalArgumentException(\"Not support Type\")"))
            tokens.append(getCmToken("    }"))
            tokens.append(getCmToken("}")).append(nl)
        }
        unionTypeSet.add(unionTypeName)
        return unionTypeName
    }
    return ""
}

public func genFunction(): Tokens {
    var ret = Tokens()
    ret.append(
        quote(
protected package tpc.UIKit.base

internal import ohos.ark_interop.*
internal import std.collection.*
internal import tpc.BasicServicesKit.*

var g_context: ?JSContext = None

let _ = JSModule.registerModule { context, _ =>
    g_context = context
}

let g_globalObjHashMap: HashMap<String, JSObject> = HashMap<String, JSObject>()

public func getGlobalObject(className: String): JSObject {
    if (g_globalObjHashMap.contains(className)) {
        return g_globalObjHashMap[className]
    }
    let ctx = g_context.getOrThrow()
    let globalThis = ctx.global
    if (!globalThis.hasProperty(className)) {
        throw IllegalArgumentException("there is no ${className} in globalThis")
    }
    g_globalObjHashMap[className] = globalThis[className].asObject()
    return g_globalObjHashMap[className]
}

public func getCtx() {
    let content = g_context ?? throw JSInteropNativeError("Component InterOp: get global context failed.")
    return content
}

public func jsObjApiCall(obj: JSObject, funcName: String, args: (JSContext) -> Array<JSValue>) {
    try {
        obj.callMethod(funcName, args(getCtx()))
    } catch (e: Exception) {
        throw BusinessException(10000, "Cannot find the ${funcName}")
    }
}

public func jsObjApiCall(obj: JSObject, funcName: String) {
    try {
    	obj.callMethod(funcName)
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

// 只支持基础类型，自定义类型手动实现
public func jsObjApiCallBackParam_0<T>(obj: JSObject, funcName: String, callback: () -> T) where T<: JSInteropType<T> {
    let callBackFunc = getCtx().function {
        ctx, callInfo =>
            callback().toJSValue(ctx)
    }
    try {
        obj.callMethod(funcName, [callBackFunc.toJSValue()])
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

// 只支持基础类型，自定义类型手动实现
public func jsObjApiCallBackParam_1<T0, T>(obj: JSObject, funcName: String, callback: (T0) -> T) where T0 <: JSInteropType<T0>, T <: JSInteropType<T>{
    let callBackFunc = getCtx().function {
        ctx, callInfo =>
            callback(T0.fromJSValue(ctx, callInfo[0])).toJSValue(ctx)
    }
    try {
        obj.callMethod(funcName, [callBackFunc.toJSValue()])
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

// 只支持基础类型，自定义类型手动实现
public func jsObjApiCallBackParam_2<T0, T1, T>(obj: JSObject, funcName: String, callback: (T0, T1) -> T)
    where T0 <: JSInteropType<T0>, T1 <: JSInteropType<T1>, T <: JSInteropType<T>{
    let callBackFunc = getCtx().function {
        ctx, callInfo =>
            callback(T0.fromJSValue(ctx, callInfo[0]), T1.fromJSValue(ctx, callInfo[1])).toJSValue(ctx)
    }
    try {
        obj.callMethod(funcName, [callBackFunc.toJSValue()])
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

// 只支持基础类型，自定义类型手动实现
public func jsObjApiCallBackParam_3<T0, T1, T2, T>(obj: JSObject, funcName: String, callback: (T0, T1, T2) -> T)
    where T0 <: JSInteropType<T0>, T1 <: JSInteropType<T1>, T2 <: JSInteropType<T2>, T <: JSInteropType<T>{
    let callBackFunc = getCtx().function {
        ctx, callInfo =>
            callback(T0.fromJSValue(ctx, callInfo[0]), T1.fromJSValue(ctx, callInfo[1]),
                T2.fromJSValue(ctx, callInfo[2])).toJSValue(ctx)
    }
    try {
        obj.callMethod(funcName, [callBackFunc.toJSValue()])
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

// 只支持基础类型，自定义类型手动实现
public func jsObjApiCallBackParam_4<T0, T1, T2, T3, T>(obj: JSObject, funcName: String, callback: (T0, T1, T2, T3) -> T)
    where T0 <: JSInteropType<T0>, T1 <: JSInteropType<T1>, T2 <: JSInteropType<T2>, T3 <: JSInteropType<T3>, T <: JSInteropType<T>{
    let callBackFunc = getCtx().function {
        ctx, callInfo =>
            callback(T0.fromJSValue(ctx, callInfo[0]), T1.fromJSValue(ctx, callInfo[1]),
                T2.fromJSValue(ctx, callInfo[2]), T3.fromJSValue(ctx, callInfo[3])).toJSValue(ctx)
    }
    try {
        obj.callMethod(funcName, [callBackFunc.toJSValue()])
    } catch (e: JSCodeError) {
        throw BusinessException(10000, "Cannot excute the ${funcName} function")
    }
}

var tempArrayList: ArrayList<JSValue> = ArrayList<JSValue>()

// 基础数据类型使用此方法转换array
public func arrayObj2JSValueBase<T>(value: Array<T>, ctx: JSContext) where T<: JSInteropType<T> {
    value |> forEach<T>({item: T => tempArrayList.append(item.toJSValue(ctx))})
    var ret = ctx.array(tempArrayList.toArray()).toJSValue()
    tempArrayList.clear()
    return ret
}

// 自定义数据类型使用此方法转换array
public func arrayObj2JSValueCustom<T>(value: Array<T>, ctx: JSContext) where T<: InteropObjectBase {
    value |> forEach<T>({item: T => tempArrayList.append(item.toJSValue(ctx))})
    var ret = ctx.array(tempArrayList.toArray()).toJSValue()
    tempArrayList.clear()
    return ret
}

// 基础数据类型使用此方法转换jsarray到cjarray
public func jsValue2CJArrayBase<T>(value: JSValue) where T <: JSInteropType<T> {
    let jsArray = value.asArray()
    let ret = ArrayList<T>()
    for (i in 0..jsArray.size) {
        ret.append(T.fromJSValue(getCtx(), jsArray[i]))
    }
    ret.toArray()
}

// 基础数据类型使用此方法转换jsarray到cjarray
public func jsValue2CJArrayCustom<T>(value: JSValue) where T <: InteropObjectBase {
    let jsArray = value.asArray()
    let ret = ArrayList<T>()
    for (i in 0..jsArray.size) {
        ret.append((T.fromJSValue(getCtx(), jsArray[i]) as T).getOrThrow())
    }
    ret.toArray()
}

// 处理 Object类型
public func string2Object(str: String): JSValue {
    let ret = getCtx().object()
    ret["key"] = str.toJSValue(getCtx())
    ret.toJSValue()
}

// 获取object的arkts类型名
public func getObjectTypeName(obj: JSObject): String {
    let constructor = obj["constructor"]
    if (constructor.isUndefined()) {
        "object"
    } else {
        String.fromJSValue(getCtx(), constructor.asObject()["name"])
    }
}
    ))
    ret
}

public func genTypeManager(): Tokens {
    var ret = Tokens()
    ret.append(
        quote(
protected package tpc.UIKit.base

public open class InteropObjectBase {
    private var jsObj: ?JSObject = None
    private let globalThis = getCtx().global

    init() {}

    protected init(obj: JSObject) {
        jsObj = obj
    }

    protected init(objName: JSKeyable) {
        if (!globalThis.hasProperty(objName)) {
            throw IllegalArgumentException("there is no ${objName} in globalThis")
        }
        jsObj = globalThis[objName].asClass().new().asObject()
    }

    protected init(objName: JSKeyable, args: Array<JSValue>) {
        if (!globalThis.hasProperty(objName)) {
            throw IllegalArgumentException("there is no ${objName} in globalThis")
        }
        jsObj = globalThis[objName].asClass().new(args).asObject()
    }

    protected func getObjectName(): String {
        let constructor = jsObj.getOrThrow()["constructor"]
        if (constructor.isUndefined()) {
            "object"
        } else {
            String.fromJSValue(getCtx(), constructor.asObject()["name"])
        }
    }

    protected func callMethod(key: JSKeyable, args: (JSContext) -> Array<JSValue>): JSValue {
        jsObj.getOrThrow().callMethod(key, args(getCtx()))
    }

    protected func callMethod(key: JSKeyable): JSValue {
        jsObj.getOrThrow().callMethod(key)
    }

    protected func getObj(): JSObject {
        jsObj.getOrThrow()
    }

    protected func getProp(key: JSKeyable): JSValue {
        jsObj.getOrThrow()[key]
    }

    protected func setProp(key: JSKeyable, value: JSValue) {
        jsObj.getOrThrow()[key] = value
    }

    protected func toJSValue(context: JSContext): JSValue {
        jsObj.getOrThrow().toJSValue()
    }

    static func fromJSValue(context: JSContext, input: JSValue): InteropObjectBase {
        InteropObjectBase(input.asObject())
    }
}
    ))
    ret
}
