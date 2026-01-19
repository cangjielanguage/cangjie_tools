// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.ast.*
import std.collection.*
import std.convert.*

public class TransEnum {
    var name: String = ""

    let keys: ArrayList<String> = ArrayList<String>()
    let values: ArrayList<String> = ArrayList<String>()
    let tips: ArrayList<String> = ArrayList<String>()
    var comment: String = ""

    let errorKV: ArrayList<String> = ArrayList<String>()

    let token = Tokens()

    var isNumber: Bool = true
    var hasInitValue: Bool = true
    var exceptionFlag: Bool = false
    var signature: String = ""
    var gconfig: GlobalConfig

    public init(enumType: EnumType, gconfig: GlobalConfig) {
        this.name = enumType.name
        this.comment = enumType.comment
        this.signature = enumType.signature()
        this.gconfig = gconfig
        let n = enumType.keyValues.size
        var lastGivenNum: Int64 = -1
        for (i in 0..n) {
            let tmp = enumType.keyValues[i].replace(" ", "").split("=")
            let k = tmp[0]
            let v = if (tmp.size == 1) {
                "" //未指定值
            } else {
                tmp[1]
            }
            if (v == "") {
                //按顺序赋一个默认值
                this.keys.add(k)
                let currentV = max(i, lastGivenNum + 1)
                this.values.add(currentV.toString())
                this.tips.add(" //todo: please check the value")
                lastGivenNum = currentV
            } else if (v.startsWith("'") && v.endsWith("'")) {
                this.keys.add(k)
                this.isNumber = false //意味着是字符串枚举
                this.values.add(v.replace("'", ""))
                this.tips.add(" //todo: please check the value")
            } else { //此时应该是有指定值的数值枚举
                let vv = if (v.endsWith(comma)) {
                    v[..v.size - 1]
                } else {
                    v
                }
                if (let Some(enumValue) <- Int64.tryParse(vv)) {
                    this.keys.add(k)
                    this.values.add(vv)
                    lastGivenNum = enumValue
                } else {
                    this.isNumber = false
                    this.keys.add(k)
                    this.values.add(v)
                }
                this.tips.add("")
            }
        }
    }

    public func generate(): Tokens {
        token.append(Tokens([Token(TokenKind.COMMENT, comment), nl]))

        token.append(addComment(signature))
        let nameToken = Token(TokenKind.IDENTIFIER, name)

        token.append(quote(public enum $(nameToken) <: ToString & Equatable<$(nameToken)> {) + nl)
        for (i in 0..keys.size) {
            let keyToken = Token(TokenKind.IDENTIFIER, keys[i])
            token.append(quote(| $(keyToken)))
            if (i != keys.size - 1) {
                token.append(nl)
            } else {
                token.append(Token(TokenKind.COMMENT, "\n"))
            }
            if (!hasInitValue) {
                //没有初始值，默认从0开始增长
                values.add(i.toString())
            }
        }
        addGetJSV()
        addParseJSV()
        addTryParseJSV()
        addToString()
        addEquatable()

        token.append(quote(}) + nl)

        checkAPIAndCommentOut(token, comment)
    }

    private func addGetJSVHelper() {
        for (i in 0..keys.size) {
            let keyToken = Token(TokenKind.IDENTIFIER, keys[i])
            let tip = Token(TokenKind.IDENTIFIER, tips[i])
            if (isNumber) {
                let valueToken = Token(TokenKind.INTEGER_LITERAL, values[i])
                token.append(quote(case $(keyToken) => $(valueToken)$(tip)) + nl)
            } else {
                token.append(quote(case $(keyToken) => $(values[i])$(tip)) + nl)
            }
        }
    }
    private func addGetJSV() {
        token.append(nl)
        let typeToken = getTypeToken()
        token.append(quote(func get(): $(typeToken) {
            match (this) {) + nl)
        addGetJSVHelper();
        token.append(quote(}) + nl)
        token.append(quote(}) + nl)
    }

    private func addParseJSVHelper() {
        for (i in 0..keys.size) {
            let keyToken = Token(TokenKind.IDENTIFIER, keys[i])
            let tip = Token(TokenKind.IDENTIFIER, tips[i])
            if (isNumber) {
                let valueToken = Token(TokenKind.INTEGER_LITERAL, values[i])
                token.append(quote(case $(valueToken) => $(keyToken)$(tip)) + nl)
            } else {
                token.append(quote(case $(values[i]) => $(keyToken)$(tip)) + nl)
            }
        }
    }

    private func addParseJSV() {
        let nameToken = Token(TokenKind.IDENTIFIER, name)
        token.append(nl)
        let typeToken = getTypeToken()
        token.append(quote(static func parse(val: $(typeToken)): $(nameToken){) + nl)

        token.append(quote(match (val) {) + nl)
        addParseJSVHelper()
        token.append(quote(case _ => throw IllegalArgumentException("unknown value ${val}")) + nl)
        token.append(quote(}) + nl)
        token.append(quote(}) + nl)
    }

    private func addTryParseJSV() {
        let nameToken = Token(TokenKind.IDENTIFIER, "?${name}")
        token.append(nl)
        let typeToken = getTypeToken(op: true)
        token.append(
            quote(static func tryParse(val: $(typeToken)): $(nameToken){
            match (val) {
                case Some(v) => parse(v)
                case None => None
            }
        }) +
                nl)
    }

    private func addToStringhelper() {
        for (i in 0..keys.size) {
            let keyToken = Token(TokenKind.IDENTIFIER, keys[i])
            token.append(quote(case $(keyToken) => $(keys[i])) + nl)
        }
    }

    private func addToString() {
        token.append(nl)
        if (!isNumber) {
            token.append(quote(public func toString(): String {
                get()
            }))
        } else {
            token.append(quote(public func toString(): String {
                match (this) {) + nl)
            addToStringhelper()
            token.append(quote(}) + nl + quote(}))
        }
        token.append(nl)
    }

    private func addEquatable() {
        token.append(nl)
        let nameToken = Token(TokenKind.IDENTIFIER, name)
        token.append(
            quote(public override operator func ==(that: $(nameToken)): Bool {
            match ((this, that)) {) + nl)
        for (i in 0..keys.size) {
            let keyToken = Token(TokenKind.IDENTIFIER, keys[i])
            token.append(quote(case ($(keyToken), $(keyToken)) => true) + nl)
        }
        token.append(quote(case _ => false) + nl)
        token.append(quote(}) + nl + quote(}) + nl + nl)
        token.append(
            quote(public override operator func !=(that: $(nameToken)): Bool {
            !(this == that)
        }))
        token.append(nl)
    }

    private func getTypeToken(op!: Bool = false) {
        match ((isNumber, op)) {
            case (true, true) => i32opToken
            case (true, false) => i32Token
            case (false, true) => stropToken
            case (false, false) => strToken
        }
    }
}
