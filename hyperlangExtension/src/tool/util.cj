// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.ast.*
import std.collection.*

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

public func getTokenWithoutNL(str: String): Tokens {
    let res = Tokens()
    return res.append(Token(TokenKind.COMMENT, str))
}

public func splitUnionParams(params: ArrayList<TsParameter>): Array<ArrayList<TsParameter>> {
    var result: Array<ArrayList<TsParameter>> = [params]
    for (i in 0..params.size) {
        let p = params[i]
        if (let TsUnion(types) <- p.ty) {
            // Do not split union type which pattern is "T|null" or "T|undefined" or "T|void"
            let nonNullTypes: ArrayList<TsType> = ArrayList<TsType>()
            for (e in types) {
                match (e) {
                    case TsNull | TsUndefined | TsVoid => ()
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

public func getTypeParametersToken(typeParameters: ArrayList<TypeParameter>): Tokens {
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

public func getTypeConstraintsToken(typeParameters: ArrayList<TypeParameter>): Tokens {
    let typeConstraintsToken = Tokens()
    if (typeParameters.size == 0) {
        return typeConstraintsToken
    } else {
        var name = "where "
        for (i in 0..typeParameters.size) {
            name += "${typeParameters[i].name} <: JSInteropType<${typeParameters[i].name}>"
            name += if (i != typeParameters.size - 1) {
                ", "
            } else {
                ""
            }
        }
        return Token(TokenKind.IDENTIFIER, name).toTokens()
    }
}