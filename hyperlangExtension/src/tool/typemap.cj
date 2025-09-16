// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.collection.*
import std.regex.*

public class TypeMap {
    private static func preprocess(type_ark: String): String {
        let tmp: String = type_ark.replace(" ", "").replace("\r", "").replace("\n", "")

        if ((tmp.startsWith("<") && (tmp.count("<") > 2 || tmp.contains("|"))) || (tmp.startsWith("{") && tmp.endsWith(
            "}")) || tmp.contains("->") || tmp.startsWith("typeof")) {
            let oneLine = Regex(#"(\r|\n|\r\n) *"#).replaceAll(type_ark, " ")
            println("WARNING: type is not supported - ${oneLine}")
            return "JSValue/* FIXME: `${oneLine}` */"
        }

        //处理lambda函数
        if (tmp.contains("=>")) {
            return handleLambda(tmp)
        }

        // handle union type which pattern is "T|null" or "T|undefined"
        if (tmp.contains("|")) {
            let arr = tmp.split("|")
            let nonNullTypes: ArrayList<String> = ArrayList<String>()
            for (e in arr) {
                if (e != "null" && e != "undefined") {
                    nonNullTypes.add(e)
                }
            }
            if (nonNullTypes.size == 1) {
                return "Option<${typeMap(nonNullTypes[0])}>"
            }
        }

        return ""
    }

    //如果输入是arkts类型，取得默认的cj类型，如果是cj类型，直接返回
    public static func typeMap(type_ark: String): String {
        let result = preprocess(type_ark)
        if (result != "") {
            return result
        }

        let tmp: String = type_ark.replace(" ", "").replace("\r", "").replace("\n", "")

        let arrMember = ArrayList<String>()
        if (type_ark.count(">") == 1) {
            arrMember.add(type_ark[0..(type_ark.indexOf("<") ?? 0)])
            arrMember.add(type_ark[((type_ark.indexOf("<") ?? 0) + 1)..(type_ark.indexOf(">") ?? 0)])
        } else if (type_ark.count(">") == 2) {
            arrMember.add(type_ark[0..(type_ark.indexOf("<") ?? 0)])
            arrMember.add(type_ark[((type_ark.indexOf("<") ?? 0) + 1)..(type_ark.lastIndexOf("<") ?? 0)])
            arrMember.add(type_ark[((type_ark.lastIndexOf("<") ?? 0) + 1)..(type_ark.indexOf(">") ?? 0)])
        }

        let ret = match (tmp) {
            // type_ark暂不考虑通配符T
            case "number" => f64
            case "string" => str
            case "void" => unit
            case "boolean" => bool
            case "bigint" => "BigInt"
            case "Uint8Array" | "ArrayBuffer" => byteArr
            case "Float32Array" => "Array<Float32>"
            case "any" | "---" | "" => any
            case "common.UIAbilityContext" | "Context" | "BaseContext" | "common.Context" | "unknown" =>
                println("WARNING: type is not supported - ${tmp}")
                return "JSValue/* FIXME: `${tmp}` */"
            case _ where tmp.startsWith(promise) => match (arrMember.size) {
                case 2 => typeMap(arrMember[1])
                case 3 => typeMap("${arrMember[1]}<${arrMember[2]}>")
                case _ =>
                    println("WARNING: type is not supported - ${tmp}")
                    return "JSValue/* FIXME: `${tmp}` */"
            }
            case _ where tmp.contains("<") && tmp.contains(">") =>
                let cj_tp = typeMap(arrMember[1])
                "${arrMember[0].replace(" ", "")}<${cj_tp}>"
            case _ where tmp.contains("[]") =>
                let cj_tp = typeMap(tmp.split("[]")[0])
                "Array<${cj_tp}>"
            case _ where (tmp.startsWith("'") && tmp.endsWith("'")) => str
            case _ =>
                if (userDefinedNames.contains(tmp)) {
                    return tmp
                } else {
                    return "JSValue/* FIXME: `${tmp}` */"
                }
        }
        ret
    }

    static func handleLambda(jsLambda: String): String {
        var transLambda = ""
        // for example:
        // ((index: number) => void)   convert to `index: number => void`
        // (index: number) => void     convert to `index: number => void`
        let tmp = jsLambda.replace("\r", "").replace("\n", "").replace("(", "").replace(")", "")
        // size of split must be 2
        let split = tmp.split("=>")
        let returnValue = typeMap(split[1])
        // lambdaContent is: `index: number`
        let lambdaContent = split[0]

        if (!lambdaContent.isEmpty()) {
            // 遍历找 < > ，把中间的逗号先改成 \n ,分割完再改回来
            let params = handleGeneric(lambdaContent)
            transLambda = transLambda + "("
            for (i in 0..params.size) {
                let param = params[i]
                if (param.isEmpty()) {
                    continue
                }
                let split = param.split(":")
                let paramName = split[0]
                let paramType = typeMap(split[1])
                if (paramName.contains("?")) {
                    transLambda = transLambda + "?${typeMap(paramType)}"
                } else {
                    transLambda = transLambda + "${typeMap(paramType)}"
                }
                if (i < params.size - 1) {
                    transLambda = transLambda + comma
                }
            }
            transLambda = transLambda + ")->" + returnValue
            transLambda
        } else {
            "()->" + returnValue
        }
    }

    static func handleGeneric(lambda: String): Array<String> {
        if (let None <- lambda.indexOf("<")) {
            return lambda.split(comma)
        }
        let byteArr = lambda.toArray()
        var leftBracketCount = 0
        for (i in 0..byteArr.size) {
            if (byteArr[i] == b'<') {
                leftBracketCount++
            }
            if (byteArr[i] == b'>') {
                leftBracketCount--
            }
            if (leftBracketCount > 0 && byteArr[i] == b',') {
                byteArr[i] = b'\n'
            }
        }
        let newStr = String.fromUtf8(byteArr)
        let split = newStr.split(comma)
        for (i in 0..split.size) {
            split[i] = split[i].replace("\n", comma)
        }
        split
    }
}
