// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.ast.*
import std.collection.*

extend TsType {
    public func toCJType(): String {
        toCJType(normalize(this))
    }

    public func toCJTypeTokens(): Tokens {
        Tokens(Token(TokenKind.IDENTIFIER, toCJType()))
    }

    /**
     * Generates a cj expression that converts a cj value to a js value.
     */
    public func toJSValue(gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens, isOptional!: Bool = false,
        initValue!: ?String = None): Tokens {
        toJSValue(normalize(this), gconfig, targetObj, ctxParam, isOptional: isOptional, initValue: initValue)
    }

    /**
     * Generates a cj expression that creates a cj value from a js value.
     */
    public func fromJSValue(gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens, isOptional!: Bool = false,
        initValue!: ?String = None): Tokens {
        fromJSValue(normalize(this), gconfig, targetObj, ctxParam, isOptional: isOptional, initValue: initValue)
    }
}

/**************************************************
 * The following functions are mutually recursive *
 **************************************************/

// preprocess the original type to reveal its real structure
private func normalize(ty: TsType): TsType {
    match (ty) {
        // arkui specific
        case TsTypeRef("common.UIAbilityContext") => TsTypeRef("JSValue/* FIXME: `${ty.toString()}` */")
        case TsTypeRef("Context") => TsTypeRef("JSValue/* FIXME: `${ty.toString()}` */")
        case TsTypeRef("BaseContext") => TsTypeRef("JSValue/* FIXME: `${ty.toString()}` */")
        case TsTypeRef("common.Context") => TsTypeRef("JSValue/* FIXME: `${ty.toString()}` */")

        // ts data structures
        case TsTypeRef("Record", arr) => normalize(TsTypeRef("HashMap", arr))

        // js data structures
        case TsTypeRef("Uint8Array") | TsTypeRef("ArrayBuffer") => TsArray(TsTypeRef("UInt8"))
        case TsTypeRef("Float32Array") => TsArray(TsTypeRef("Float32"))

        // general case
        case TsTypeRef(x, arr) => TsTypeRef(x, arr.map(normalize))
        case TsUnion(arr) => TsUnion(arr.map(normalize))
        case TsIntersection(arr) => TsIntersection(arr.map(normalize))
        case TsTuple(arr) => TsTuple(arr.map(normalize))
        case TsFunction(r, params, t) => TsFunction(normalize(r),
            params.map {p => TsParameter(p.name, normalize(p.ty), p.optional)}, t)
        case TsArray(t) => TsArray(normalize(t))
        case _ => ty
    }
}

private func toCJType(ty: TsType): String {
    match (ty) {
        case TsNumber => f64
        case TsString => str
        case TsVoid => unit
        case TsBoolean => bool
        case TsBigInt => "BigInt"
        case TsAny => any

        case TsFunction(ret, params, generics) =>
            if (generics.size > 0) {
                // cj doesn't support type-node level type parameter
                // type parameters should be extracted to the outer interface definition
                println("WARNING: type parameters are not supported in type node - ${ty}")
            }
            func opt(p: TsParameter) {
                if (p.optional) {
                    quote(?)
                } else {
                    quote()
                }
            }
            "(${params.map {it => "${it.name}: ${opt(it)}${toCJType(it.ty)}"} |> collectString<String>(delimiter: ", ")}) -> ${toCJType(ret)}" +
                if (generics.size > 0) {
                "/* FIXME: `${ty}` */"
            } else {
                ""
            }
        case TsArray(elemTy) => "Array<${toCJType(elemTy)}>"
        case TsOptional(elemTy) => "Option<${toCJType(elemTy)}>"
        case TsTuple(elems) => "(${elems.map(toCJType) |> collectString<String>(delimiter: ", ")})"
        case TsUnion(elems) =>
            let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
            for (e in elems) {
                match (e) {
                    case TsNull | TsUndefined => ()
                    case _ => nonNullTypes.add(e)
                }
            }
            if (nonNullTypes.size == 1) {
                return "Option<${toCJType(nonNullTypes[0])}>"
            }
            // FIXME: General union types need further processing.
            println("WARNING: type is not supported - ${ty}")
            "JSValue/* FIXME: `${ty}` */"
        case TsTypeRef(x) =>
            if (userDefinedNames.contains(x)) {
                return x
            } else {
                return "JSValue/* FIXME: `${x}` */"
            }
        case TsTypeRef("Promise", _) => "JSValue/* FIXME: `${ty}` */"
        case TsTypeRef(x, arr) =>
            var hasJSValueTy = false
            for (ty in arr) {
                if (toCJType(ty).startsWith("JSValue")) {
                    hasJSValueTy = true
                    break
                }
            }
            if (userDefinedNames.contains(x) && hasJSValueTy) {
                "JSValue"
            } else {
                "${x}<${arr.map(toCJType) |> collectString<String>(delimiter: ", ")}>"
            }

        case _ =>
            println("WARNING: type is not supported - ${ty}")
            "JSValue/* FIXME: `${ty}` */"
    }
}

private func genTo(tsType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens, typeName: ?String,
    isOptional: Bool, arr!: ?Array<TsType> = None): Tokens {
    let isEnum = if (let Some(v) <- typeName) {
        gconfig.isEnum(v)
    } else {
        false
    }
    let isHashMap = if (let Some(v) <- typeName) {
        v == "HashMap"
    } else {
        false
    }
    let get = if (isEnum) {
        quote(.get())
    } else {
        quote()
    }
    let res = if (isOptional) {
        quote(.getOrDefault({=> $(ctxParam).object().toJSValue()}))
    } else {
        quote()
    }
    let toValue = if (toCJType(tsType).startsWith("JSValue")) {
        res
    } else {
        quote(.toJSValue($(ctxParam)))
    }
    if (isHashMap) {
        let secondParam = toCJType(arr.getOrThrow()[1])
        let res = Token(TokenKind.IDENTIFIER, "hashmap2Record<${secondParam}>")
        quote($(res)($(ctxParam), $(targetObj)$(get), {context, i => i.toJSValue(context)}))
    } else {
        quote($(targetObj)$(get)$(toValue))
    }
}

private func wrapJSFunc(ret: TsType, gconfig: GlobalConfig, ctxParam: Tokens, params: Array<TsParameter>, f: Tokens) {
    // function.toJS would involve params.fromJs
    let innerCtx = quote(ctx)
    let callInfo = quote(info)
    // 1. transform each parameter from js value
    let transformParams = params.map {
        p, i =>
        let varName = Token(TokenKind.IDENTIFIER, "p${i}")
        let pValue = quote($(callInfo)[$(i)])
        let pFromJS = fromJSValue(p.ty, gconfig, pValue, innerCtx)
        quote(let $(varName) = $(pFromJS))
    } |> reduce {v: Tokens, elem: Tokens => v + nl + elem}

    let paramList = Token(TokenKind.IDENTIFIER,
        params.map {_: TsParameter, i: Int64 => "p${i}"} |> collectString<String>(delimiter: ", "))
    let call = quote($(f)($(paramList)))
    // avoid unused parameter
    let paramInfos = if (params.isEmpty()) {
        quote($(innerCtx), _)
    } else {
        quote($(innerCtx), $(callInfo))
    }
    // 3. transform return value to js as necessary
    let callAndHandleRet = if (let TsVoid <- ret) {
        quote($(call)
            $(innerCtx).undefined().toJSValue())
    } else {
        let retToJs = toJSValue(ret, gconfig, quote(ret), innerCtx)
        quote(let ret = $(call)
            $(retToJs))
    }
    // handle parameters as necessary
    if (params.isEmpty()) {
        quote($(ctxParam).function({$(paramInfos) =>
            $(callAndHandleRet)
        }).toJSValue())
    } else {
        quote($(ctxParam).function({$(paramInfos) =>
            $(transformParams.getOrThrow())
            $(callAndHandleRet)
        }).toJSValue())
    }
}

/**
 * Generates a cj expression that converts a cj value to a js value.
 */
private func toJSValue(tsType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens,
    isOptional!: Bool = false, initValue!: ?String = None): Tokens {
    let optional = isOptional && initValue.isNone()
    match (tsType) {
        case TsBigInt => quote($(ctxParam).bigint($(targetObj)).toJSValue($(ctxParam)))
        case TsTypeRef(typeName) => genTo(tsType, gconfig, targetObj, ctxParam, Some(typeName), optional)
        case TsTypeRef(typeName, array) => genTo(tsType, gconfig, targetObj, ctxParam, Some(typeName), optional,
            arr: array)
        case TsUnion(elems) =>
            let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
            for (e in elems) {
                match (e) {
                    case TsNull | TsUndefined => ()
                    case _ => nonNullTypes.add(e)
                }
            }
            if (nonNullTypes.size == 1) {
                return toJSValue(nonNullTypes[0], gconfig, targetObj, ctxParam, isOptional: true)
            }
            return genTo(tsType, gconfig, targetObj, ctxParam, None, optional)
        case TsFunction(ret, params, _) =>
            if (optional) {
                let wrapped = wrapJSFunc(ret, gconfig, ctxParam, params, quote(v))
                quote(match ($(targetObj)) {
                    case None => $(ctxParam).undefined.toJSValue()
                    case Some(v) => $(wrapped)
                })
            } else {
                wrapJSFunc(ret, gconfig, ctxParam, params, targetObj)
            }
        case TsArray(elemType) => toJSArray(elemType, gconfig, targetObj, ctxParam)
        case _ => genTo(tsType, gconfig, targetObj, ctxParam, None, optional)
    }
}

private func toJSArray(elemType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens): Tokens {
    let innerType = elemType.toCJTypeTokens()
    match (elemType) {
        case TsNumber | TsString | TsBoolean => quote(toJSArray<$(innerType)>($(ctxParam), $(targetObj)))
        case _ =>
            let elemCtx = quote(ctx)
            let elemValue = quote(val)
            let elemToJS = toJSValue(elemType, gconfig, elemValue, elemCtx)
            let transformElements = quote({$(elemCtx): JSContext, $(elemValue): $(innerType) => $(elemToJS)})
            quote(toJSArray<$(innerType)>($(ctxParam), $(targetObj), $(transformElements)))
    }
}

private func genFrom(tsType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens,
    isOptional!: Bool = false, initValue!: ?String = None) {
    let cjType = tsType.toCJTypeTokens()
    let opt = isOptional && initValue.isNone()
    let tsTypeStr = tsType.toString()
    let fromValue = if (toCJType(tsType).startsWith("JSValue")) {
        quote($(targetObj))
    } else {
        quote(.fromJSValue($(ctxParam),$(targetObj)))
    }
    if (gconfig.isEnum(tsTypeStr)) {
        let parse = if (opt) {
            quote(tryParse)
        } else {
            quote(parse)
        }
        let elemType = match (gconfig.moduleMeta.symbols.get(tsTypeStr)) {
            case Some("enum-string") => quote(String)
            case Some(_) | None => quote(Int32)
        }
        let valueType = if (opt) {
            quote(Option<$(elemType)>)
        } else {
            elemType
        }
        return quote($(cjType).$(parse)($(valueType)$(fromValue)))
    }
    if (toCJType(tsType).startsWith("JSValue")) {
        quote($(fromValue))
    } else {
        if (opt) {
            quote(Option<$(cjType)>$(fromValue))
        } else {
            quote($(cjType)$(fromValue))
        }
    }
}

private func genFunction(ret: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens,
    params: Array<TsParameter>, isOptional!: Bool = false): Tokens {
    let innerCtx = quote(ctx)

    // "name0: type0, name1: type1, ..."
    let paramList = (params
        .map {
            p =>
            let pVar = Token(TokenKind.IDENTIFIER, p.name)
            let opt = if (p.optional) {
                quote(?)
            } else {
                quote()
            }
            quote($(pVar): $(opt)$(p.ty.toCJTypeTokens()))
        } |> reduce {l: Tokens, r: Tokens => l + quote(,) + r})
        .getOrDefault {=> quote()}

    // "arg0, arg1, ..."
    let args = (params.map {
        _, i =>
        let pVar = Token(TokenKind.IDENTIFIER, "arg${i}")
        Tokens(pVar)
    } |> reduce {
        l: Tokens, r: Tokens => l + quote(,) + r
    })

    // "let argX = nameX toJs"
    let prepareArgs = params.map {
        p, i =>
        let pVar = Token(TokenKind.IDENTIFIER, "arg${i}")
        let pName = Token(TokenKind.IDENTIFIER, p.name)
        let argToJs = toJSValue(p.ty, gconfig, Tokens(pName), innerCtx, isOptional: p.optional)
        quote(let $(pVar) = $(argToJs))
    } |> fold(quote()) {l, r => l + nl + r}

    let call = if (params.size > 0) {
        quote($(targetObj).asFunction().call([$(args.getOrThrow())]))
    } else {
        quote($(targetObj).asFunction().call())
    }
    let noReturn = if (let TsVoid <- ret) {
        true
    } else {
        false
    }
    let innerBody = if (noReturn) {
        call
    } else {
        let retFromJS = fromJSValue(ret, gconfig, quote(ret), innerCtx)
        quote(let ret = $(call)
            $(retFromJS))
    }

    // avoid unused parameter
    let innerCtxParam = if (noReturn && params.isEmpty()) {
        quote(_)
    } else {
        innerCtx
    }

    let funcValue = quote({$(paramList) =>
        checkThreadAndCall<$(ret.toCJTypeTokens())>($(ctxParam), {$(innerCtxParam) => $(prepareArgs)
            $(innerBody)
        })
    })
    if (isOptional) {
        quote(if ($(targetObj).isUndefined()) {
            None
        } else {
            $(funcValue)
        })
    } else {
        funcValue
    }
}

private func fromJSValue(tsType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens,
    isOptional!: Bool = false, initValue!: ?String = None): Tokens {
    match (tsType) {
        case TsBigInt => quote($(targetObj).asBigInt($(ctxParam)).toBigInt())
        case TsArray(elemType) => fromJSArray(elemType, gconfig, targetObj, ctxParam, isOptional: isOptional,
            initValue: initValue)
        case TsTuple(elems) =>
            var result = quote(\()
            var index = 0
            for (e in elems) {
                let indexQuote = Token(TokenKind.IDENTIFIER, "${index}")
                let targetObjWithIndex = targetObj + quote(.asObject()[$(indexQuote)])
                result += genFrom(e, gconfig, targetObjWithIndex, ctxParam, isOptional: isOptional, initValue: initValue)
                if (index != elems.size - 1) {
                    result += quote(,)
                }
                index += 1
            }
            result += quote(\))
            return result
        case TsTypeRef("HashMap", array) =>
            let secondParam = toCJType(array[1])
            let res = Token(TokenKind.IDENTIFIER, "record2Hashmap<${secondParam}>")
            let secondParamToken = Token(TokenKind.IDENTIFIER, secondParam)
            quote($(res)($(ctxParam), $(targetObj), {context, i => $(secondParamToken).fromJSValue(context, i)}))
        case TsUnion(elems) =>
            let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
            for (e in elems) {
                match (e) {
                    case TsNull | TsUndefined => ()
                    case _ => nonNullTypes.add(e)
                }
            }
            if (nonNullTypes.size == 1) {
                return fromJSValue(nonNullTypes[0], gconfig, targetObj, ctxParam, isOptional: true)
            }
            genFrom(tsType, gconfig, targetObj, ctxParam, isOptional: isOptional, initValue: initValue)
        case TsFunction(ret, params, _) => genFunction(ret, gconfig, targetObj, ctxParam, params, isOptional: isOptional)
        case _ => genFrom(tsType, gconfig, targetObj, ctxParam, isOptional: isOptional, initValue: initValue)
    }
}

private func fromJSArray(elemType: TsType, gconfig: GlobalConfig, targetObj: Tokens, ctxParam: Tokens,
    isOptional!: Bool = false, initValue!: ?String = None) {
    let opt = isOptional && initValue.isNone()
    let from = if (opt) {
        quote(fromJSArrayOption)
    } else {
        quote(fromJSArray)
    }
    match (elemType) {
        case TsNumber | TsString | TsBoolean => quote($(from)<$(elemType.toCJTypeTokens())>($(ctxParam), $(targetObj)))
        case _ =>
            let elemCtx = quote(ctx)
            let elemValue = quote(val)
            let elemToJS = fromJSValue(elemType, gconfig, elemValue, elemCtx)
            let transformElements = quote({$(elemCtx): JSContext, $(elemValue): JSValue => $(elemToJS)})
            quote($(from)<$(elemType.toCJTypeTokens())>($(ctxParam), $(targetObj), $(transformElements)))
    }
}
