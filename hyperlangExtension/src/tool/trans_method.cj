// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.ast.*
import std.collection.*
import std.regex.*

public class TransMethod {
    var arkName = ""
    var arkModifiers = ArrayList<String>()
    var arkParaName = ArrayList<String>()
    var arkParaType = ArrayList<String>()
    var arkRetType = ""

    var cjName = ""
    var cjModifiers = ArrayList<String>()
    var cjParaName = ArrayList<String>()
    var cjParaType = ArrayList<String>()
    var cjRetType = ""

    var moduleFullName = ""
    var memberClassName = ""
    var isStatic = false

    var exceptionFlag = false
    var optionFlag = false

    var origFuncSignature: String = ""
    var comment: String = ""
    var methodType = MethodType()
    var hasAsyncCallback = false
    var callbackInnerType = ""
    var isPromise = false
    var promiseNeedGen = false
    var gconfig: GlobalConfig

    //根据单个arkts的函数描述，默认地构造一个cj的描述
    public init(methodType: MethodType, memberClassName: String, gconfig: GlobalConfig) {
        this.origFuncSignature = methodType.signature()
        this.arkName = methodType.name.replace(" ", "")
        this.cjName = if (methodType.isConstructor) {
            "init "
        } else {
            dedupFuncName(arkName)
        }
        this.arkModifiers = methodType.modifiers
        this.comment = methodType.comment
        this.methodType = methodType
        this.moduleFullName = gconfig.moduleFullName
        this.memberClassName = memberClassName
        this.gconfig = gconfig
        for (mdf in arkModifiers) {
            match (mdf) {
                case "public" | "private" | "protected" => this.cjModifiers.add(mdf) //只关注可见性修饰符
                case _ => ()
            }
        }
        if (origFuncSignature.contains(promise)) {
            isPromise = true
            promiseNeedGen = funcRetPromiseOnly(methodType)
        }
        if (cjModifiers.size == 0) {
            this.cjModifiers.add("public") //默认为public
        }

        if (arkModifiers.contains("StaticKeyword")) {
            this.cjModifiers.add("static")
            this.isStatic = true
        }

        for (item in methodType.parameters) {
            let paraName = item.name //arkts参数名
            let paraType = item.ty.toString() //arkts参数类型

            if (isAsyncCallback(paraType)) {
                hasAsyncCallback = true
                callbackInnerType = TypeMap.typeMap(paraType[14..(paraType.size - 1)])
            }
            this.arkParaName.add(paraName)
            this.arkParaType.add(paraType)
            optionFlag = item.optional
            this.cjParaName.add(dedupVarName(paraName))
            this.cjParaType.add(item.ty.toCJType())
        }
        let arkR = methodType.returns
        this.arkRetType = arkR
        this.cjRetType = TypeMap.typeMap(arkR)
        if (promiseNeedGen) {
            callbackInnerType = cjRetType
        }
    }

    private static func parseParam(j: Int64, k: Int64, params: String) {
        let raw = params[j + 1..k].split(",")
        let result = ArrayList<String>()
        var i = 0
        while (i < raw.size) {
            let pType = if (let Some(j) <- raw[i].indexOf(':')) {
                raw[i][j + 1..]
            } else {
                raw[i]
            }
            if (pType.contains("Record<")) {
                result.add("${pType}${raw[i + 1]}")
                i++
            } else {
                result.add(pType)
            }
            i++
        }
        return result
    }

    // parse a type string like Callback<T1, T2, ...> or (a1: T1, a2: T2, ...)=>TR,
    // hacky, not robustly implemented, should use a parser instead
    // @deprecated
    static func parseCallbackParamTypes(t: String): ArrayList<String> {
        if (let Some(i) <- t.indexOf("=>")) {
            let params = t[..i - 1]
            if (let Some(j) <- params.indexOf('(')) {
                if (let Some(k) <- params.lastIndexOf(')')) {
                    return parseParam(j, k, params)
                }
            }
        } else if (t.contains("Callback")) {
            if (let Some(i) <- t.indexOf('<')) {
                if (let Some(j) <- t.lastIndexOf('>')) {
                    return ArrayList(t[i + 1..j].split(","))
                }
            }
        }
        ArrayList<String>()
    }

    public func generateConstructor(className: String): Tokens {
        handleConstructor(className)
    }

    public func getArkTSFuncName(): String {
        if (methodType.isConstructor) {
            "init"
        } else {
            this.arkName
        }
    }

    //生成非成员函数
    public func generateFunc(): Tokens {
        generateAsyncFunc()
    }

    public func processSyncFunc(funcName: Token, paramToken: Tokens, prefix: ArrayList<Modifier>,
        transparamToken: Tokens): Tokens {
        let moduleName = gconfig.requireModuleName
        let modulePrefix = gconfig.requireModulePrefixToken
        let retValueToken = getSyncRetToken(this.cjRetType)
        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)
        let call = if (transparamToken.size == 0) {
            quote(jsModule.callMethod($(getArkTSFuncName())))
        } else {
            quote(jsModule.callMethod($(getArkTSFuncName()), $(transparamToken)))
        }
        quote(jsModule.callMethod($(getArkTSFuncName()), $(transparamToken)))
        let template: Tokens = quote(
                $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                    basicJsSkeleton<$(retTypeToken)> {
                        ret, exception, sync =>
                        let ctx = getGlobalContext()
                        let jsModule = getJSModule($(moduleName), $(modulePrefix))
                        let jsRet = $(call)
                        $(retValueToken)
                        sync.dec()
                    }
                }
            )
        template
    }

    public func processAsyncFunc(funcName: Token, paramToken: Tokens, prefix: ArrayList<Modifier>,
        transparamToken: Tokens): Tokens {
        let moduleName = gconfig.requireModuleName
        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)
        let third_param = Tokens()
        if (this.methodType.params.size == 0) {
            third_param.append(quote(noArg))
        } else {
            third_param.append(quote({=> [$(transparamToken)]}))
        }
        let forth_param = getAsyncRetToken(this.cjRetType, retTypeToken, sync: true)

        let apiToken = if (gconfig.isOhos) {
            Token(TokenKind.IDENTIFIER, "ohosGlobalApiCallAsync")
        } else {
            Token(TokenKind.IDENTIFIER, "hmsGlobalApiCallAsync")
        }

        let staticConstructor = if (this.isStatic) {
            Token(TokenKind.IDENTIFIER, "getClassConstructorObj(\"${moduleName}\", \"${this.memberClassName}\"),")
        } else {
            Token(TokenKind.IDENTIFIER, "")
        }

        let template: Tokens = quote(
                $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                    let ctx = getGlobalContext()
                    $(apiToken)<$(retTypeToken)>($(staticConstructor) $(moduleName), $(getArkTSFuncName()),$(third_param))$(forth_param)
                }
            )
        template
    }

    // @deprecated
    public func generateSyncFunc(): Tokens {
        let result = Tokens()
        result.append(mergeCommentToken(comment, origFuncSignature))
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let paramToken = getParamToken()
        let transparamToken = getTransParamToken()

        if (isSync(this.origFuncSignature)) {
            result.append(processSyncFunc(funcName, paramToken, prefix, transparamToken))
        } else {
            let template: Tokens = processAsyncFunc(funcName, paramToken, prefix, transparamToken)
            if (this.origFuncSignature.contains("AsyncCallback")) {
                result.append(addComment(template.toString(), reason: "AsyncCallback is not included"))
            } else {
                result.append(template)
            }
        }

        checkAPIAndCommentOut(result, comment)
    }

    public func generateAsyncFunc(): Tokens {
        let result = Tokens()
        if (isPromise && !promiseNeedGen) {
            return result
        } else if (promiseNeedGen) {
            handlePromise(result, isMember: false)
            return result
        }
        if (isEventAPI(methodType)) {
            return handleEvent(isMember: false)
        }
        result.append(mergeCommentToken(comment, origFuncSignature))
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let ctxToken = quote(ctx)
        let paramToken = paramListToCJ()
        let typeParameters = getTypeParametersToken()
        let transparamToken = (parametersToJS(ctxToken) |> reduce {l, r => l + commaToken + r}).getOrDefault {
            => quote()
        }

        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)

        let moduleName = gconfig.requireModuleName

        let third_param = Tokens()
        if (this.methodType.params.size == 0) {
            third_param.append(quote(emptyArg))
        } else {
            third_param.append(quote({$(ctxToken) => [$(transparamToken)]}))
        }
        let forth_param = getAsyncRetToken(this.cjRetType, retTypeToken)

        let apiToken = if (gconfig.isOhos) {
            Token(TokenKind.IDENTIFIER, "ohosGlobalApiCall")
        } else {
            Token(TokenKind.IDENTIFIER, "hmsGlobalApiCall")
        }

        var staticConstructor = Token(TokenKind.IDENTIFIER, "")
        if (this.isStatic) {
            staticConstructor = Token(TokenKind.IDENTIFIER, "getClassConstructorObj(\"${moduleName}\", \"${this.memberClassName}\"), ")
        }

        var cbToken = quote()
        if (hasAsyncCallback) {
            cbToken = handleAsyncCallBack() + nl
        }

        let template: Tokens = quote(
            $(prefix) func $(funcName)$(typeParameters)($(paramToken)):$(retTypeToken) {
                $(cbToken)$(apiToken)<$(retTypeToken)>($(staticConstructor)$(moduleName), $(getArkTSFuncName()), $(third_param))$(forth_param)
            }
        )
        result.append(template)

        checkAPIAndCommentOut(result, comment)
    }

    //生成成员函数
    public func generateMemberFunc(): Tokens {
        generateAsyncMemberFunc()
    }

    // @deprecated
    public func generateSyncMemberFunc(): Tokens {
        let result = Tokens()
        result.append(mergeCommentToken(comment, origFuncSignature))
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let paramToken = getParamToken()
        let transparamToken = getTransParamToken()
        let typeParameters = getTypeParametersToken()

        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)

        if (isSync(this.origFuncSignature)) {
            let retValueToken = getSyncRetToken(this.cjRetType)
            let call = if (transparamToken.size == 0) {
                quote(arkts_object.callMethod($(getArkTSFuncName())))
            } else {
                quote(arkts_object.callMethod($(getArkTSFuncName()), $(transparamToken)))
            }
            let template: Tokens = quote(
                $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                    basicJsSkeleton<$(retTypeToken)> {
                        ret, exception, sync =>
                        let ctx = getGlobalContext()
                        let jsRet = $(call)
                        $(retValueToken)
                        sync.dec()
                    }
                }
            )
            result.append(template)
        } else {
            let third_param = Tokens()
            if (this.methodType.params.size == 0) {
                third_param.append(quote(noArg))
            } else {
                third_param.append(quote({=> [$(transparamToken)]}))
            }

            let forth_param = getAsyncRetToken(this.cjRetType, retTypeToken, sync: true)

            let template: Tokens = quote(
                $(prefix) func $(funcName)$(typeParameters)($(paramToken)):$(retTypeToken) {
                    let ctx = getGlobalContext()
                    arktsObjApiCallAsync<$(retTypeToken)>(arkts_object, $(getArkTSFuncName()), $(third_param)) $(forth_param)
                }
            )
            if (this.origFuncSignature.contains("AsyncCallback")) {
                result.append(addComment(template.toString(), reason: "AsyncCallback is not included"))
            } else {
                result.append(template)
            }
        }
        checkAPIAndCommentOut(result, comment)
    }

    public func generateAsyncMemberFunc(): Tokens {
        let result = Tokens()
        if (isPromise && !promiseNeedGen) {
            return result
        } else if (promiseNeedGen) {
            handlePromise(result, isMember: true)
            return result
        }
        if (isEventAPI(methodType)) {
            return handleEvent(isMember: true)
        }
        result.append(mergeCommentToken(comment, origFuncSignature))
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let ctxToken = quote(ctx)
        let paramToken = paramListToCJ()
        let transparamToken = (parametersToJS(ctxToken) |> reduce {l, r => l + commaToken + r}).getOrDefault {
            => quote()
        }

        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)

        let third_param = Tokens()
        if (this.methodType.params.size == 0) {
            third_param.append(quote(emptyArg))
        } else {
            third_param.append(quote({$(ctxToken) => [$(transparamToken)]}))
        }
        let moduleName = gconfig.requireModuleName

        let forth_param = getAsyncRetToken(this.cjRetType, retTypeToken)

        var cbToken = quote()
        if (hasAsyncCallback) {
            cbToken = handleAsyncCallBack() + nl
        }

        let staticConstructor = if (this.isStatic) {
            Token(TokenKind.IDENTIFIER, "getClassConstructorObj(\"${moduleName}\", \"${this.memberClassName}\"),")
        } else {
            Token(TokenKind.IDENTIFIER, "")
        }
        // static member func can be regarded as global func
        let memberToken = if (this.isStatic) {
            Token(TokenKind.IDENTIFIER, "")
        } else {
            Token(TokenKind.IDENTIFIER, "arkts_object,")
        }

        let template: Tokens = quote(
            $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                $(cbToken)jsObjApiCall<$(retTypeToken)>($(staticConstructor)$(memberToken)$(getArkTSFuncName()), $(third_param)) $(forth_param)
            }
        )
        result.append(template)

        checkAPIAndCommentOut(result, comment)
    }

    public func isSync(sign: String): Bool {
        for (keyword in syncKeyword) {
            if (sign.contains(keyword)) {
                return false
            }
        }
        return true
    }

    public func paramListToCJ(): Tokens {
        (this
            .methodType
            .parameters |> map {
                p =>
                let pName = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(p.name))
                let pType = p.ty.toCJTypeTokens()
                if (p.optional) {
                    quote($(pName)!: Option<$(pType)> = None)
                } else {
                    quote($(pName): $(pType))
                }
            } |> reduce {l, r => l + commaToken + r})
            .getOrDefault {=> quote()}
    }

    public func getTypeParametersToken(typeParameters!: ArrayList<TypeParameter> = this.methodType.typeParameters): Tokens {
        let typeParametersToken = Tokens()
        if (typeParameters.isEmpty()) {
            return typeParametersToken
        }

        typeParametersToken.append(Token(TokenKind.LT))
        for (i in 0..typeParameters.size) {
            typeParametersToken.append(Token(TokenKind.IDENTIFIER, typeParameters[i].name))
            if (i < typeParameters.size - 1) {
                typeParametersToken.append(commaToken)
            }
        }
        typeParametersToken.append(Token(TokenKind.GT))
        return typeParametersToken
    }

    // @deprecated
    public func getParamToken(params!: ArrayList<String> = this.methodType.params, isAssignDefaultValue!: Bool = true): Tokens {
        let paramToken = Tokens()

        for (i in 0..params.size) {
            if (params[i].isEmpty()) {
                continue
            }
            let paramName = params[i].replace(": ", ":")
            let arkP = paramName.split(":")
            var paraName = arkP[0] //arkts参数名
            var paraType = ""
            if (paramName.contains("=>")) {
                paraType = TypeMap.typeMap(paramName[arkP[0].size + 1..paramName.size])
            } else if (arkP.size > 2) {
                var temp = ""
                for (i in 1..arkP.size) {
                    temp = temp + ":" + arkP[i]
                }
                paraType = TypeMap.typeMap(temp)
            } else if (arkP.size == 2) {
                paraType = TypeMap.typeMap(arkP[1])
            } else {
                paraType = TypeMap.typeMap("any")
            }

            if (paraName.contains("?")) {
                paraName = paraName.replace("?", "")
                paraName = checkCangjieKeyword(paraName)
                let paraNameToken = Token(TokenKind.IDENTIFIER, paraName)
                let paraTypeToken = Token(TokenKind.IDENTIFIER, paraType)
                if (isAssignDefaultValue) {
                    paramToken.append(quote($(paraNameToken)!: ?$(paraTypeToken) = None))
                } else {
                    paramToken.append(quote($(paraNameToken)!: ?$(paraTypeToken)))
                }
            } else if (isAsyncCallback(paraType)) {
                paramToken.append(
                    Token(TokenKind.IDENTIFIER, paraName) + Token(TokenKind.COLON) + Token(TokenKind.IDENTIFIER,
                        transAsyncCallback(callbackInnerType)))
            } else {
                paraName = checkCangjieKeyword(paraName)
                paramToken.append(
                    Token(TokenKind.IDENTIFIER, paraName) + Token(TokenKind.COLON) + Token(TokenKind.IDENTIFIER,
                        paraType))
            }
            if (i < params.size - 1) {
                paramToken.append(commaToken)
            }
        }
        return paramToken
    }

    // params: list of (<name>: <type>) pairs
    // @deprecated
    public static func paramListToJS(params: ArrayList<String>, gconfig: GlobalConfig, ctxName!: String = "ctx"): Tokens {
        let transparamToken = Tokens()

        for (i in 0..params.size) {
            if (params[i].isEmpty()) {
                continue
            }
            transparamToken.append(paramToJS(params[i], ctxName, gconfig))
            if (i < params.size - 1) {
                transparamToken.append(commaToken)
            }
        }
        return transparamToken
    }

    // param: a (<name>: <type>) pair
    // @deprecated
    static func paramToJS(param: String, ctxName: String, gconfig: GlobalConfig): Tokens {
        let arkP = param.replace(" ", "").split(":")
        var paraName = arkP[0] //arkts参数名

        // If js does not have type, we used "any" before type check implemented
        var paraType = arkP[1]
        if (arkP.size < 2) {
            paraType = "any"
        }

        var optional = false
        if (paraName.contains("?")) {
            optional = true
            paraName = paraName.replace("?", "")
        }
        paraName = checkCangjieKeyword(paraName)
        let nameToken = Token(TokenKind.IDENTIFIER, paraName)
        let ctx = Token(TokenKind.IDENTIFIER, ctxName)
        if (isAsyncCallback(paraType)) {
            quote(jsCallback)
        } else if (paraType == AbilityContext) {
            quote(getJSContext(mainThreadRuntime.getOrThrow(), $(nameToken)))
        } else {
            if (optional) {
                if (gconfig.isEnum(paraType)) {
                    quote(($(nameToken)?.get()).toJSValue($(ctx)))
                } else if (isArray(paraType)) {
                    let innerType = getArrayInnerType(paraType)
                    let typeToken = Token(TokenKind.IDENTIFIER, innerType)
                    let innerToken = getInnerTypeToken(innerType, gconfig)
                    quote(toJSArray<$(typeToken)>\($(ctx), $(nameToken), {$(ctx), item => $(innerToken)}\))
                } else {
                    quote(($(nameToken)?.toJSValue($(ctx))) ?? $(ctx).undefined().toJSValue())
                }
            } else {
                if (gconfig.isEnum(paraType)) {
                    quote($(nameToken).get().toJSValue($(ctx)))
                } else if (isArray(paraType)) {
                    let innerType = getArrayInnerType(paraType)
                    let typeToken = Token(TokenKind.IDENTIFIER, innerType)
                    let innerToken = getInnerTypeToken(innerType, gconfig)
                    quote(toJSArray<$(typeToken)>\($(ctx), $(nameToken), {$(ctx), item => $(innerToken)}\))
                } else if (isBigInt(paraType)) {
                    quote(ctx.bigint($(nameToken)).toJSValue(ctx))
                } else if (isUint8Array(paraType)) {
                    quote(toJSArray<UInt8>($(ctx), $(nameToken), {$(ctx), item => item.toJSValue($(ctx))}))
                } else if (isNumberArray(paraType)) {
                    quote(toJSArray<Float64>($(ctx), $(nameToken), {$(ctx), item => item.toJSValue($(ctx))}))
                } else {
                    quote($(nameToken).toJSValue($(ctx)))
                }
            }
        }
    }

    public func parametersToJS(ctx: Tokens): Array<Tokens> {
        this
            .methodType
            .parameters
            .toArray()
            .map {
                p =>
                let pName = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(p.name))
                p.ty.toJSValue(gconfig, quote($(pName)), ctx, isOptional: p.optional)
            }
    }

    // @deprecated
    public func getTransParamToken(): Tokens {
        paramListToJS(this.methodType.params, gconfig)
    }

    // 暂不考虑二维数组
    // @deprecated
    static func getInnerTypeToken(innerType: String, gconfig: GlobalConfig): Tokens {
        if (gconfig.isEnum(innerType)) {
            quote(item.get().toJSValue(ctx))
        } else {
            quote(item.toJSValue(ctx))
        }
    }

    // @deprecated
    func getSyncRetToken(retType: String): Tokens {
        let retValueToken = Tokens()
        let cjType = TypeMap.typeMap(retType)
        let retToken = Token(TokenKind.IDENTIFIER, cjType)
        if (cjType == unit) {
            retValueToken.append(quote(ret.value = ))
            retValueToken.append(Token(TokenKind.IDENTIFIER, "()"))
            return retValueToken
        } else if (gconfig.isEnum(cjType)) {
            retValueToken.append(quote(ret.value = ))
            retValueToken.append(quote($(retToken).parse(Int32.fromJSValue(ctx, jsRet))))
        } else if (isArray(cjType)) {
            let innerType = getArrayInnerType(cjType)
            let innerToken = Token(TokenKind.IDENTIFIER, innerType)
            retValueToken.append(
                quote(let jsArr = jsRet.asArray()
            ret.value = Array<$(innerToken)>(jsArr.size) {
                index =>
                    $(innerToken).fromJSValue(ctx, jsArr[index])
            }))
        } else {
            retValueToken.append(quote(ret.value = ))
            retValueToken.append(quote($(retToken).fromJSValue(ctx, jsRet)))
        }
        retValueToken
    }

    func getAsyncRetToken(retType: String, retTypeToken: Token, sync!: Bool = false): Tokens {
        let cjType = TypeMap.typeMap(retType)
        if (interopTypes.contains(cjType)) {
            return quote()
        }
        let valueToken = if (sync) {
            Token(TokenKind.IDENTIFIER, "info[0]")
        } else {
            Token(TokenKind.IDENTIFIER, "info")
        }
        let retToken = Tokens()
        if (gconfig.isEnum(cjType)) {
            retToken.append(
                quote({
                ctx, info => $(retTypeToken).parse(Int32.fromJSValue(ctx, $(valueToken)))
            }))
        } else if (isArray(cjType)) {
            let innerType = getArrayInnerType(cjType)
            let innerToken = Token(TokenKind.IDENTIFIER, innerType)
            let fromValue = if (innerType.startsWith("JSValue")) {
                quote()
            } else {
                quote(.fromJSValue(ctx, retArr[index]))
            }
            retToken.append(
                quote({
                ctx, info =>
                    let retArr = $(valueToken).asArray()
                    Array<$(innerToken)>(retArr.size) {
                        index => $(innerToken)$(fromValue)
                    }
            }))
        } else {
            let innerToken = if (quote($(retTypeToken)).toString().startsWith("JSValue")) {
                quote($(valueToken))
            } else {
                quote($(retTypeToken).fromJSValue(ctx, $(valueToken)))
            }
            retToken.append(quote({
                ctx, info => $(innerToken)
            }))
        }
        retToken
    }

    func getCbRetToken(retType: String): Tokens {
        let retTypeToken = Token(TokenKind.IDENTIFIER, TypeMap.typeMap(retType))
        let cjType = TypeMap.typeMap(retType)
        if (interopTypes.contains(cjType)) {
            return quote()
        }
        let retToken = Tokens()
        if (gconfig.isEnum(cjType)) {
            retToken.append(
                quote({
                ctx, info => $(retTypeToken).parse(Int32.fromJSValue(ctx, info))
            }))
        } else if (isArray(cjType)) {
            let innerType = getArrayInnerType(cjType)
            let innerToken = Token(TokenKind.IDENTIFIER, TypeMap.typeMap(innerType))
            retToken.append(
                quote({
                ctx, info =>
                    let retArr = info.asArray()
                    Array<$(innerToken)>(retArr.size) {
                        index => $(innerToken).fromJSValue(ctx, retArr[index])
                    }
            }))
        } else if (isBigInt(cjType)) {
            retToken.append(quote({
                ctx, info => info.asBigInt(ctx).toBigInt()
            }))
        } else {
            retToken.append(quote({
                ctx, info => $(retTypeToken).fromJSValue(ctx, info)
            }))
        }
        retToken
    }

    func handleAsyncCallBack(): Tokens {
        let innerToken = Token(TokenKind.IDENTIFIER, callbackInnerType)
        let retToken = getCbRetToken(callbackInnerType)
        quote(let ctx = getMainContext()
            let jsCallback = asyncCallbackWrapper<$(innerToken)>(ctx, callback) $(retToken))
    }

    func handlePromise(result: Tokens, isMember!: Bool) {
        result.append(mergeCommentToken(comment, origFuncSignature))
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let innerType = Token(TokenKind.IDENTIFIER, callbackInnerType)
        let paramToken = quote(callback: AsyncCallback<$(innerType)>)
        if (this.methodType.params.size > 0) {
            paramToken.append(commaToken)
        }
        paramToken.append(getParamToken())

        let transparamToken = (parametersToJS(quote(ctx)) |> reduce {l, r => l + commaToken + r}).getOrDefault {
            => quote()
        }

        var transParam = quote({ctx => [$(transparamToken)]})
        if (this.methodType.params.size == 0) {
            transParam = quote(emptyArg)
        }

        var apiToken = Token(TokenKind.IDENTIFIER, "hmsGlobalApiCallPromise")
        if (gconfig.isOhos) {
            apiToken = Token(TokenKind.IDENTIFIER, "ohosGlobalApiCallPromise")
        }

        let resolveRetToken = getAsyncRetToken(this.cjRetType, innerType)

        var staticConstructor = Token(TokenKind.IDENTIFIER, "")
        if (this.isStatic) {
            staticConstructor = Token(TokenKind.IDENTIFIER, "getClassConstructorObj(\"${gconfig.requireModuleName}\", \"${this.memberClassName}\"),")
        }

        let template = if (isMember) {
            if (this.isStatic) {
                quote(
                    $(prefix) func $(funcName)($(paramToken)): Unit {
                        jsObjApiCallPromise<$(innerType)>($(staticConstructor) $(getArkTSFuncName()), callback, $(transParam)) $(resolveRetToken)
                    }
                )
            } else {
                quote(
                    $(prefix) func $(funcName)($(paramToken)): Unit {
                        jsObjApiCallPromise<$(innerType)>($(staticConstructor) arkts_object, $(getArkTSFuncName()), callback, $(transParam)) $(resolveRetToken)
                    }
                )
            }
        } else {
            quote(
                $(prefix) func $(funcName)($(paramToken)): Unit {
                    $(apiToken)<$(innerType)>($(staticConstructor) $(gconfig.requireModuleName), $(getArkTSFuncName()), callback, $(transParam)) $(resolveRetToken)
                }
            )
        }

        result.append(template)

        checkAPIAndCommentOut(result, comment)
    }

    private func processOnEvent(callbackInnerTypes: ArrayList<String>, isMember: Bool, apiCallParamsToJS: Tokens,
        paramToken: Tokens, firstParamNameToken: Token, callbackParamToken: Token, staticConstructor: Token,
        apiToken: Token) {
        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)
        let moduleName = gconfig.requireModuleName
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let wrapperArgs = Tokens()
        let callbackParams = Tokens()
        for (i in 0..callbackInnerTypes.size) {
            let argName = Token(TokenKind.IDENTIFIER, "arg" + i.toString())
            let argTypeName = TypeMap.typeMap(callbackInnerTypes[i])
            let argTypeToken = Token(TokenKind.IDENTIFIER, argTypeName)
            wrapperArgs.append(
                match (gconfig.moduleMeta.symbols.get(argTypeName)) {
                    case Some("enum-string") => quote(let $(argName) = $(argTypeToken).parse(String.fromJSValue(__ctx, info[$(i)])))
                    case Some("enum-numeric") => quote(let $(argName) = $(argTypeToken).parse(Int32.fromJSValue(__ctx, info[$(i)])))
                    case _ => quote(let $(argName) = $(argTypeToken).fromJSValue(__ctx, info[$(i)]))
                })
            callbackParams.append(argName)
            if (i < callbackInnerTypes.size - 1) {
                wrapperArgs.append(nl)
                callbackParams.append(commaToken)
            }
        }
        let call = if (isMember) {
            quote(jsObjApiCall<$(retTypeToken)>($(staticConstructor) arkts_object, $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS), jsCallback]}))
        } else {
            quote($(apiToken)<$(retTypeToken)>($(staticConstructor) $(moduleName), $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS), jsCallback]}))
        }
        quote(
                    $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                        let key = $(firstParamNameToken).toString()
                        if (callbackManager.findCallbackObject(key, $(callbackParamToken)).isSome()) {
                            return
                        }
                        let jsCallback = checkThreadAndCall<JSValue>(getMainContext()) {
                            __ctx => __ctx.function {
                                __ctx: JSContext, info: JSCallInfo =>
                                $(wrapperArgs)
                                $(callbackParamToken).invoke($(callbackParams))
                                __ctx.undefined().toJSValue()
                            }.toJSValue()
                        }
                        callbackManager.put(key, ($(callbackParamToken), jsCallback))
                        $(call)
                    }
                )
    }

    private func processOffEvent(isMember: Bool, apiCallParamsToJS: Tokens, paramToken: Tokens,
        firstParamNameToken: Token, callbackParamToken: Token, staticConstructor: Token, apiToken: Token) {
        let retTypeToken = Token(TokenKind.IDENTIFIER, this.cjRetType)
        let moduleName = gconfig.requireModuleName
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList
        let funcName: Token = Token(TokenKind.IDENTIFIER, checkCangjieKeyword(cjName))
        let ifCall = if (isMember) {
            quote(jsObjApiCall<$(retTypeToken)>($(staticConstructor) arkts_object, $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS), jsCallback ?? __ctx.undefined().toJSValue()]}))
        } else {
            quote($(apiToken)<$(retTypeToken)>($(moduleName), $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS), jsCallback ?? __ctx.undefined().toJSValue()]}))
        }
        let elseCall = if (isMember) {
            quote(jsObjApiCall<$(retTypeToken)>($(staticConstructor) arkts_object, $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS)] }))
        } else {
            quote($(apiToken)<$(retTypeToken)>($(moduleName), $(getArkTSFuncName()), {__ctx => [$(apiCallParamsToJS)] }))
        }
        quote(
                    $(prefix) func $(funcName)($(paramToken)):$(retTypeToken) {
                        let __ctx = getMainContext()
                        let key = $(firstParamNameToken).toString()
                        if (let Some(v) <- $(callbackParamToken)) {
                            let jsCallback = callbackManager.findCallbackObject(key, v, remove: true)
                            if (jsCallback.isNone()) {
                                throw BusinessException(401, "Callback does not exist.")
                            }
                            $(ifCall)
                        } else {
                            if (callbackManager.isCallbackExist(key)) {
                                callbackManager.clear(key)
                                $(elseCall)
                            }
                        }
                    }
                )
    }

    func processOnAndOff(callbackInnerTypes: ArrayList<String>, isMember: Bool, apiCallParamsToJS: Tokens,
        paramToken: Tokens, firstParamNameToken: Token, callbackParamToken: Token) {
        let moduleName = gconfig.requireModuleName
        let staticConstructor = if (this.isStatic) {
            Token(TokenKind.IDENTIFIER, "getClassConstructorObj(\"${moduleName}\", \"${this.memberClassName}\"),")
        } else {
            Token(TokenKind.IDENTIFIER, "")
        }
        let apiToken = if (gconfig.isOhos) {
            Token(TokenKind.IDENTIFIER, "ohosGlobalApiCall")
        } else {
            Token(TokenKind.IDENTIFIER, "hmsGlobalApiCall")
        }
        if (arkName == "on") {
            processOnEvent(callbackInnerTypes, isMember, apiCallParamsToJS, paramToken, firstParamNameToken,
                callbackParamToken, staticConstructor, apiToken)
        } else if (arkName == "off") {
            processOffEvent(isMember, apiCallParamsToJS, paramToken, firstParamNameToken, callbackParamToken,
                staticConstructor, apiToken)
        } else {
            throw IllegalArgumentException("Unexpected: event api with a name other than 'on' and 'off'")
        }
    }

    private func getCallbackParamName() {
        var callbackParamName = this.methodType.params[this.methodType.params.size - 1]
        if (let Some(i) <- callbackParamName.indexOf(':')) {
            if (let Some(j) <- callbackParamName.indexOf('?')) {
                if (j < i) {
                    callbackParamName = checkCangjieKeyword(callbackParamName[..j])
                } else {
                    callbackParamName = checkCangjieKeyword(callbackParamName[..i])
                }
            } else {
                callbackParamName = checkCangjieKeyword(callbackParamName[..i])
            }
        }
        return callbackParamName
    }

    func handleEvent(isMember!: Bool): Tokens {
        let result = Tokens()
        result.append(mergeCommentToken(comment, origFuncSignature))

        var apiCallParams = ArrayList(this.methodType.params[0..this.methodType.params.size - 1])
        // replace first param name to "key"
        var firstParamName = apiCallParams[0]
        if (let Some(i) <- apiCallParams[0].indexOf(':')) {
            apiCallParams[0] = "key" + apiCallParams[0][i..]
            firstParamName = checkCangjieKeyword(firstParamName[..i])
        }
        let firstParamNameToken = Token(TokenKind.IDENTIFIER, firstParamName)

        let callbackParamToken = Token(TokenKind.IDENTIFIER, getCallbackParamName())

        let params = ArrayList(this.methodType.params)
        let lastParam = params[params.size - 1]
        let callbackParamType = if (let Some(i) <- lastParam.indexOf(':')) {
            lastParam[i + 1..]
        } else {
            ""
        }
        let callbackInnerTypes = parseCallbackParamTypes(callbackParamType)
        // replace last parameter type with CallbackNArgument<>
        let callbackVarName = if (let Some(i) <- lastParam.indexOf(':')) {
            lastParam[..i]
        } else {
            ""
        }
        params[params.size - 1] = callbackVarName + ": Callback" + callbackInnerTypes.size.toString() + "Argument" + if (callbackInnerTypes
            .size > 0) {
            let s = StringBuilder("<")
            s.append(callbackInnerTypes[0])
            for (i in 1..callbackInnerTypes.size) {
                s.append(", ")
                s.append(callbackInnerTypes[i])
            }
            s.append(">")
            s.toString()
        } else {
            ""
        }
        let paramToken = getParamToken(params: params)
        let apiCallParamsToJS = quote($(paramListToJS(apiCallParams, gconfig, ctxName: "__ctx")))

        result.append(
            processOnAndOff(callbackInnerTypes, isMember, apiCallParamsToJS, paramToken, firstParamNameToken,
                callbackParamToken))
        checkAPIAndCommentOut(result, comment)
        result
    }

    func handleConstructor(className: String): Tokens {
        let result = Tokens()
        result.append(mergeCommentToken(comment, origFuncSignature))
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        let moduleName = gconfig.arkTSMoudleName
        let modulePrefix = gconfig.requireModulePrefixToken
        let paramToken = getParamToken()
        let args = paramListToJS(this.methodType.params, gconfig, ctxName: "__ctx")
        let quotedClass = quote($(className))

        result.append(
            if (gconfig.is3rd) {
                quote(
                $(prefix) init($(paramToken)) {
                    arkts_object = checkThreadAndCall<JSObject>(getMainContext()) {
                        __ctx =>
                            let clazz = getClassConstructorObj($(moduleName), $(quotedClass)).asClass(__ctx)
                            clazz.new($(args)).asObject()
                    }
                })
            } else {
                quote(
                $(prefix) init($(paramToken)) {
                    arkts_object = checkThreadAndCall<JSObject>(getMainContext()) {
                        __ctx =>
                            let module = getJSModule(__ctx, $(moduleName), $(modulePrefix))
                            let clazz = getClassConstructorObj($(moduleName), $(quotedClass)).asClass(__ctx)
                            clazz.new($(args)).asObject()
                    }
                })
            })
        result
    }

    func generateEmptyParamConstructor(className: String): Tokens {
        let result = Tokens()
        let quotedClass = quote($(className))
        let moduleName = gconfig.arkTSMoudleName

        result.append(
            quote(
            public init() {
                arkts_object = checkThreadAndCall<JSObject>(getMainContext()) {
                    __ctx =>
                        let clazz = getClassConstructorObj($(moduleName), $(quotedClass)).asClass(__ctx)
                        clazz.new().asObject()
                }
            }))
        result
    }

    public func getParamNameToken(params!: ArrayList<String> = this.methodType.params): Tokens {
        // todo 是否可以改用cjParaName
        let paramToken = Tokens()

        for (i in 0..params.size) {
            if (params[i].isEmpty()) {
                continue
            }
            let arkP = params[i].replace(" ", "").split(":")
            var paraName = arkP[0] //arkts参数名
            if (paraName.contains("?")) {
                paraName = paraName.replace("?", "")
                paraName = checkCangjieKeyword(paraName)
            }
            paramToken.append(Token(TokenKind.IDENTIFIER, paraName + ".toJSValue(ctx)"))
            if (i < params.size - 1) {
                paramToken.append(commaToken)
            }
        }
        return paramToken
    }

    public func generateComponentConstructor(className: String, isContainedBase!: Bool = false): Tokens {
        let result = Tokens()
        let prefix: ArrayList<Modifier> = cjModifiers |> map {mdf => Modifier(Token(TokenKind.IDENTIFIER, mdf))} |>
            collectArrayList

        var paramToken = getParamToken()
        if (isContainedBase) {
            if (paramToken.toString().isEmpty()) {
                paramToken.append(quote(child: () -> Unit))
            } else {
                if (paramToken.toString().contains("!:")) {
                    paramToken = quote(child: () -> Unit, ).concat(paramToken)
                } else {
                    paramToken.append(quote(, child: () -> Unit))
                }
            }
        }
        let paramNameTokens = getParamNameToken()
        let call = Tokens()
        if (this.methodType.params.size == 0) {
            call.append(getTokenWithoutNL("jsObjApiCall(jsObj.getOrThrow(), \"create\")"))
        } else {
            call.append(
                getTokenWithoutNL("jsObjApiCall(jsObj.getOrThrow(), \"create\", { ctx => [${paramNameTokens}]})"))
        }
        if (isContainedBase) {
            call.append(nl)
            call.append(quote(mychild = child))
        }

        result.append(
            quote(
            $(prefix) init($(paramToken)) {
                jsObj = getGlobalObject($(className))
                $(call)
            }
            ))
        result
    }

    public func convertOptionalArgs(params!: ArrayList<String> = this.methodType.params): (Tokens, ArrayList<String>) {
        let transparamToken = Tokens()
        let args = ArrayList<String>()
        for (i in 0..params.size) {
            let param = params[i]
            if (param.isEmpty()) {
                continue
            }
            let arkP = param.replace(" ", "").split(":")
            var paraName = arkP[0] //arkts参数名
            if (paraName.contains("?")) {
                paraName = paraName.replace("?", "")
                transparamToken.append(
                    Token(TokenKind.IDENTIFIER,
                        "let " + paraName + "Arg = if (let Some(v) <- " + checkCangjieKeyword(paraName) +
                            ") { v } else { getCtx().object() }"))
                paraName += "Arg"
            }
            args.add(paraName)
        }
        return (transparamToken, args)
    }

    public func getTypeFromPromise(uiRetType: String): String {
        if (uiRetType == "") {
            return ""
        }
        let left_regex = Regex("^.*?<")
        let right_regex = Regex(">[^>]*$")
        var res = left_regex.replace(uiRetType, "")
        res = right_regex.replace(res, "")
        return res
    }

    public func getArgsToken(argNames: ArrayList<String>, ctxName!: String = "ctx"): Tokens {
        let argToken = Tokens()
        for (i in 0..argNames.size) {
            argToken.append(Token(TokenKind.IDENTIFIER, argNames[i] + ".toJSValue(" + ctxName + ")"))
            if (i < argNames.size - 1) {
                argToken.append(commaToken)
            }
        }
        argToken
    }

    public func getParameters(params!: ArrayList<String> = this.methodType.params): String {
        let args = ArrayList<String>()
        for (i in 0..params.size) {
            let param = params[i]
            if (param.isEmpty()) {
                continue
            }
            let arkP = param.replace(" ", "").split(":")
            var paraName = arkP[0] //arkts参数名
            let paraType = TypeMap.typeMap(arkP[1]) //参数分类
            var optionalSymbol = ": "
            if (paraName.contains("?")) {
                paraName = paraName.replace("?", "")
                optionalSymbol = "!: ?"
            }
            args.add(", " + checkCangjieKeyword(paraName) + optionalSymbol + paraType)
        }
        "callback: (" + getTypeFromPromise(this.cjRetType) + ") -> Unit" + (args |> collectString<String>())
    }
}
