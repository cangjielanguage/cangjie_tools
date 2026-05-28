// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.ast.*
import std.sort.*
import std.collection.*

public class TransObject {
    var arkMethods: ArrayList<MethodType> = ArrayList<MethodType>()
    var arkParentsMethods: ArrayList<MethodType> = ArrayList<MethodType>()
    var arkProperties: ArrayList<Property> = ArrayList<Property>()
    var arkParentsProperties: ArrayList<Property> = ArrayList<Property>()
    var arkComment: String = ""
    var arkName: String = ""
    var cjModifiers: ArrayList<String> = ArrayList<String>(["public"])
    var arkParents: ArrayList<String> = ArrayList<String>()
    var arkParentsCategories: ArrayList<String> = ArrayList<String>()
    var arkChildren: ArrayList<String> = ArrayList<String>()
    var tokens = Tokens() // 总tokens
    var signature = ""
    var arkModifiers: ArrayList<String> = ArrayList<String>()
    var typeParameters: ArrayList<TypeParameter> = ArrayList<TypeParameter>()
    public var arkCategory: String = "interface"
    public var cjCategory: String = "class"
    var gconfig: GlobalConfig
    let unionTypeSet: ArrayList<String> = ArrayList<String>()

    public init(objectType: ObjectType, gconfig: GlobalConfig) {
        this.arkProperties = objectType.properties
        this.arkParentsProperties = objectType.parentsProperties
        sort<Property>(this.arkProperties, stable: true)
        sort<Property>(this.arkParentsProperties, stable: true)
        this.arkMethods = objectType.methods
        this.arkParentsMethods = objectType.parentsMethods
        this.arkComment = objectType.comment
        this.arkName = objectType.name
        this.arkParents = objectType.parents
        for (parent in arkParents) {
            if (objectType.nameSpace.classes.contains(parent)) {
                this.arkParentsCategories.add("class")
            } else {
                this.arkParentsCategories.add("interface")
            }
        }
        if (objectType.nameSpace.childrenMap.contains(arkName)) {
            this.arkChildren = objectType.nameSpace.childrenMap[arkName]
        }
        this.signature = objectType.signature()
        this.gconfig = gconfig
        this.arkModifiers = objectType.modifiers
        this.typeParameters = objectType.typeParameters
    }

    public func generateClassTypeParameters(): String {
        let templateArkName = if (this.typeParameters.size == 0) {
            ""
        } else {
            var name = "<"
            for (i in 0..typeParameters.size) {
                name += typeParameters[i].name + if (i != typeParameters.size - 1) {
                    ", "
                } else {
                    ">"
                }
            }
            name
        }
        templateArkName
    }

    public func generateClassTypeParametersConstraintsToken(): Tokens {
        if (this.typeParameters.size == 0) {
            quote()
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
            Token(TokenKind.IDENTIFIER, name).toTokens()
        }
    }

    private func genPropertiesWithOutArktsObject(totalProperties: ArrayList<Property>, property: Tokens, fromJSValueItem: Tokens,
        getPropertyItem: Tokens) {
        for (i in 0..totalProperties.size) {
            let arkProp = totalProperties[i]
            var proName = Token(TokenKind.IDENTIFIER, arkProp.cjKey)
            let proType = arkProp.typeNode.toCJTypeTokens()
            let tsType = arkProp.typeNode
            let ctxToken = Tokens(Token(TokenKind.IDENTIFIER, "context"))

            let isLast = i >= totalProperties.size - 1
            genfieldSign(property, arkProp, proName, proType, last: isLast)

            // generate toJS statement for this field
            getPropertyItem.append(
                if (arkProp.isOptional) {
                    let toJs = tsType.toJSValue(gconfig, quote(v), ctxToken)
                    quote(if$(space)(let Some(v) <- $(proName)) {
                            obj[$(arkProp.propKey)] = $(toJs)
                        })
                } else {
                    let toJs = tsType.toJSValue(gconfig, quote($(proName)), ctxToken)
                    quote(obj[$(arkProp.propKey)]= $(toJs))
                })

            // generate fromJS statement for this field
            if (arkProp.isOptional) {
                fromJSValueItem.append(quote($(proName): ))
            }
            let fromJs = tsType.fromJSValue(gconfig, quote(obj[$(arkProp.propKey)]), ctxToken,
                isOptional: arkProp.isOptional, initValue: arkProp.defValue)
            fromJSValueItem.append(fromJs)

            if (!isLast) {
                getPropertyItem.append(nl)
                fromJSValueItem.append(commaToken + nl)
            }
        }
    }

     // translate the interface that only has properties and no methods.
    func genPropWithOutArktsObject(property: Tokens, jsValueConvertToken: Tokens, templateClassNameToken: Token) {
        let classNameToken = Token(TokenKind.IDENTIFIER, arkName)
        let getPropertyItem = Tokens()
        let fromJSValueItem = Tokens()

        // merge properties
        let totalProperties = ArrayList<Property>()
        for (p in arkParentsProperties) {
            if (!p.isOptional) {
                totalProperties.add(p)
            }
        }
        for (p in arkProperties) {
            if (!p.isOptional) {
                totalProperties.add(p)
            }
        }
        // Cangjie syntax: function non-named parameters must appear before named parameters.
        for (p in arkParentsProperties) {
            if (p.isOptional) {
                totalProperties.add(p)
            }
        }
        for (p in arkProperties) {
            if (p.isOptional) {
                totalProperties.add(p)
            }
        }

        // generate properties
        genPropertiesWithOutArktsObject(totalProperties, property, fromJSValueItem, getPropertyItem)

        jsValueConvertToken.append(quote(
                public open func toJSValue(context: JSContext): JSValue {
                    let obj = context.object()
                    $(getPropertyItem)
                    obj.toJSValue()
                }

                public static func fromJSValue(context: JSContext, input: JSValue): $(templateClassNameToken) {
                    let obj = input.asObject()
                    $(classNameToken)(
                        $(fromJSValueItem)
                    )
                })
        )
    }

    private func genConstructor(tm: TransMethod, constructorToken: Tokens) {
        var superCall = false
        if (arkParentsMethods.size > 0 || arkParentsCategories.contains("class")) {
            superCall = true
        }
        constructorToken.append(tm.generateConstructor(arkName, superCall))
    }

    private func processArkMethod(constructorToken: Tokens, methodToken: Tokens) {
        var isPublicType = false
        for (i in 0..arkMethods.size) {
            let arkMethod = arkMethods[i]
            if (arkMethod.name == "") {
                isPublicType = true
                continue
            }
            /**
             * If parent has already translated this method, skip it. else compile error: cannot override function.
             * interface Drivable {
             *     start(): void;
             * }
             * ->
             * public func start(): Unit {
             *     jsObjApiCall < Unit >( arkts_object, "start", EMPTY_ARG)
             * }
             *
             * declare class Car1 implements Drivable {
             *     start(): void;
             * }
             * ->
             * public func start(): Unit {
             *     jsObjApiCall < Unit >( arkts_object, "start", EMPTY_ARG)
             * }
             */
            var hasParentAlreadyTranslate = false
            for (arkParentsMethod in arkParentsMethods) {
                if (arkMethod == arkParentsMethod) {
                    hasParentAlreadyTranslate = true
                    break
                }
            }
            if (hasParentAlreadyTranslate) {
                println("WARNING: ${arkName} parent has already translated ${arkMethod.name} method, skip it")
                continue
            }
            var params = splitUnionParams(arkMethod.parameters)
            for (param in params) {
                arkMethod.parameters = param
                let tm = TransMethod(arkMethod, arkName, gconfig)
                if (arkMethod.isConstructor) {
                    genConstructor(tm, constructorToken)
                } else if (arkMethod.isStatic()) {
                    methodToken.append(tm.generateFunc())
                } else {
                    methodToken.append(tm.generateMemberFunc())
                }
            }
        }
        return isPublicType
    }

    func genMethodOfClass(constructorToken: Tokens, methodToken: Tokens, templateClassNameToken: Token) {
        var isPublicType = false
        if (arkMethods.size > 0 || arkParentsMethods.size > 0 || this.arkCategory == "class") {
            let classNameToken = Token(TokenKind.IDENTIFIER, arkName)
            isPublicType = processArkMethod(constructorToken, methodToken)
            var modifiers = if (this.cjModifiers.contains("open")) {
                "public open "
            } else if (arkParents.size > 0) { 
                "public "
            } else {
                ""
            }
            let toJSValueToken = quote(
                $(getTokenWithoutNL(modifiers + "func"))toJSValue(_: JSContext): JSValue {
                    arkts_object.toJSValue()
                }
            )
            methodToken.append(toJSValueToken)

            modifiers = if (this.cjModifiers.contains("open") || arkParents.size > 0) {
                "public "
            } else {
                ""
            }
            let fromJSValueToken = quote(
                $(getTokenWithoutNL(modifiers + "static"))func fromJSValue(_: JSContext, input: JSValue): $(templateClassNameToken) {
                    $(classNameToken)\(input.asObject()\)
                })
            methodToken.append(fromJSValueToken)
        }
        return isPublicType
    }

    private func generateSuperCallWithOutArktsObject() {
        if (arkParentsProperties.size == 0) {
            return Tokens()
        }
        var res = Tokens(Token(TokenKind.IDENTIFIER, "super("))
        for (i in 0..arkParentsProperties.size) {
            let arkProp = arkParentsProperties[i]
            let isLast = i >= arkParentsProperties.size - 1
            if (arkProp.isOptional) {
                res.append(Token(TokenKind.IDENTIFIER, arkProp.cjKey + ": "))
            }
            res.append(Token(TokenKind.IDENTIFIER, arkProp.cjKey))
            if (!isLast) {
                res.append(Token(TokenKind.IDENTIFIER, ","))
            }
        }
        res.append(Token(TokenKind.IDENTIFIER, ")"))
        return res
    }

    private func genSuperCallWithArktsObject(totalProperties: ArrayList<Property>, fromJSValueItem: Tokens) {
        for (i in 0..totalProperties.size) {
            let arkProp = totalProperties[i]
            var proName = Token(TokenKind.IDENTIFIER, arkProp.cjKey)
            let tsType = arkProp.typeNode
            let ctxToken = Tokens(Token(TokenKind.IDENTIFIER, "getMainContext()"))

            let isLast = i >= totalProperties.size - 1
            // generate fromJS statement for this field
            if (arkProp.isOptional) {
                fromJSValueItem.append(quote($(proName): ))
            }
            let fromJs = tsType.fromJSValue(gconfig, quote(arkts_object[$(arkProp.propKey)]), ctxToken,
                isOptional: arkProp.isOptional, initValue: arkProp.defValue)
            fromJSValueItem.append(fromJs)

            if (!isLast) {
                fromJSValueItem.append(commaToken + nl)
            }
        }
    }

    private func generateTokenWithoutArktsObject(modifiersTokens: Tokens, templateClassNameToken: Token, parent: Tokens,
        classConstraintsToken: Tokens): Tokens {
        let property = Tokens()
        let jsValueConvertToken = Tokens()

        genPropWithOutArktsObject(property, jsValueConvertToken, templateClassNameToken)

        let varDeclare = Tokens()
        for (arkProp in arkProperties) {
            let prefix = if (arkProp.isReadOnly()) {
                Token(TokenKind.IDENTIFIER, "let")
            } else {
                Token(TokenKind.IDENTIFIER, "var")
            }
            var proName = Token(TokenKind.IDENTIFIER, arkProp.cjKey)
            let propType = arkProp.typeNode.toCJTypeTokens()
            if (arkProp.isOptional) {
                let (typeToken, defToken) = if (let Some(def) <- arkProp.defValue) {
                    (propType, Token(TokenKind.IDENTIFIER, def))
                } else {
                    (Tokens(Token(TokenKind.IDENTIFIER, "Option<${propType}>")), noneToken)
                }
                varDeclare.append(quote(public $(prefix) $(proName): $(typeToken) = $(defToken)))
            } else {
                varDeclare.append(quote(public $(prefix) $(proName): $(propType)))
            }
            varDeclare.append(nl)
        }

        let assignment = Tokens()
        for (arkProp in arkProperties) {
            var proName = Token(TokenKind.IDENTIFIER, arkProp.cjKey)
            assignment.append(quote(this.$(proName) = $(proName)))
            assignment.append(nl)
        }

        tokens.append(quote(
                $(modifiersTokens) class $(templateClassNameToken)$(parent)$(classConstraintsToken) {
                ))

        if (hasEventAPI(arkMethods)) {
           tokens.append(declareCallbackManager())
        }
        if (!arkProperties.isEmpty()) {
            tokens.append(varDeclare)
        }
        if (arkProperties.size == 0 || arkProperties.size == 1) {
            tokens.append(quote(protected init($(property)) {) + nl)
        } else {
            tokens.append(quote(protected init(
                                    $(property)) {) + nl)
        }
        if (!arkParentsProperties.isEmpty()) {
            tokens.append(generateSuperCallWithOutArktsObject() + nl)
        }
        if (!arkProperties.isEmpty()) {
            tokens.append(assignment)
        }
        tokens.append(quote(}
            $(jsValueConvertToken)
            }
        ))
        tokens
    }

    private func generateTokenWithArktsObject(modifiersTokens: Tokens, templateClassNameToken: Token, parent: Tokens,
        classConstraintsToken: Tokens): Tokens {
        let classNameToken = Token(TokenKind.IDENTIFIER, arkName)
        let eventManager = if (hasEventAPI(arkMethods)) {
            declareCallbackManager()
        } else {
            quote()
        }

        tokens.append(quote(
                $(modifiersTokens) class $(templateClassNameToken)$(parent)$(classConstraintsToken) {
                    $(eventManager)))

        // the main constructor
         if (arkParents.size == 0 || (!arkParentsCategories.contains("class") && arkParentsMethods.size == 0)) {
            tokens.append(quote(protected $(classNameToken)(var arkts_object: JSObject)))
         } else {
            tokens.append(quote(protected $(classNameToken)(arkts_object: JSObject)))
         }

        if (arkParents.size > 0 ) {
            // The parent class has arkts_object attribute
            if (arkParentsCategories[0] == "class" || arkParentsMethods.size > 0) {
                tokens.append(quote({ super(arkts_object) }))
            // The parent class is an interface that only has properties and no methods(without arkts_object attribute).
            } else if (arkParentsCategories[0] == "interface") {
            /* The generated code is as follows
                super(Float64.fromJSValue(getMainContext(), arkts_object["p"]))
            */
            let superTokens = Tokens()
            genSuperCallWithArktsObject(arkParentsProperties, superTokens)
            tokens.append(quote({
                super($(superTokens))
                }))
            }
        } else {
            tokens.append(quote({}))
        }

        let methodToken = Tokens()
        let constructorToken = Tokens()
        let propertyDeclare = Tokens()
        genMethodOfClass(constructorToken, methodToken, templateClassNameToken)
        for (i in 0..arkProperties.size) {
            genPropDeclare(i, propertyDeclare)
        }
        tokens.append(quote(
            $(constructorToken)
            $(propertyDeclare)
            $(methodToken)
        }))
        tokens
    }

    public func generate(): Tokens {
        let canExport = this.arkModifiers.contains("FirstContextualKeyword") || this
            .arkModifiers
            .contains("AbstractKeyword") || this.arkCategory == "interface" || this.arkChildren.size > 0
        if (canExport) {
            this.cjModifiers.add("open")
        }

        let headToken = Tokens(Token(TokenKind.COMMENT, arkComment), nl)
        tokens.append(headToken)
        tokens.append(addComment(arkCategory + signature))

        let typeParameter = generateClassTypeParameters()
        let templateClassNameToken = Token(TokenKind.IDENTIFIER, arkName + typeParameter)
        let classConstraintsToken = generateClassTypeParametersConstraintsToken()
        var parent = Tokens()
        if (arkParents.size > 0) {
            let parentstoken = Token(TokenKind.IDENTIFIER, arkParents[0])
            parent = Token(TokenKind.UPPERBOUND) + parentstoken
        }

        let modifiersTokens = Tokens()
        for (modifier in this.cjModifiers) {
            modifiersTokens.append(Token(TokenKind.IDENTIFIER, modifier))
        }

        let isInterfaceWithoutMethod = arkMethods.size == 0 && arkParentsMethods.size == 0 && this.arkCategory == "interface"
        if (isInterfaceWithoutMethod) {
            generateTokenWithoutArktsObject(modifiersTokens, templateClassNameToken, parent, classConstraintsToken)
        } else {
            generateTokenWithArktsObject(modifiersTokens, templateClassNameToken, parent, classConstraintsToken)
        }

        checkAPIAndCommentOut(tokens, arkComment)
    }

    // options!: Option<AutoGenType0Ininheritances> = None, public var nested!: Option<AutoGenType1Ininheritances> = None
    func genfieldSign(property: Tokens, arkProp: Property, proName: Token, proType: Tokens, last!: Bool = false) {
        if (arkProp.isOptional) {
            let (typeToken, defToken) = if (let Some(def) <- arkProp.defValue) {
                (proType, Token(TokenKind.IDENTIFIER, def))
            } else {
                (Tokens(Token(TokenKind.IDENTIFIER, "Option<${proType}>")), noneToken)
            }
            property.append(quote($(proName)!: $(typeToken) = $(defToken)))
        } else {
            property.append(quote($(proName): $(proType)))
        }
        if (!last) {
            property.append(commaToken + nl)
        }
    }

    func genSet(arkProp: Property, propAccess: Tokens, toValue: Tokens): Tokens {
        if (arkProp.isReadOnly()) {
            quote()
        } else {
            quote(
                set(v) {
                    checkThreadAndCall<Unit>(getMainContext()) {
                        ctx: JSContext => $(propAccess) = $(toValue)
                    }
                }
            )
        }
    }

    func genPropDeclare(i: Int64, propertyDeclare: Tokens) {
        let arkProp = arkProperties[i]
        // cj does not support prop declare of same name between parent and child
        for (arkParentsProperty in this.arkParentsProperties) {
            if (arkProp.propKey == arkParentsProperty.propKey) {
                println("WARNING: ${arkName} parent has already translated ${arkProp.propKey} prop, skip it")
                return
            }
        }
        propertyDeclare.append(Token(TokenKind.COMMENT, arkProp.comment))
        let propName = Token(TokenKind.IDENTIFIER, arkProp.propKey)
        let propType = arkProp.typeNode.toCJTypeTokens()
        let propAccess = if (arkProp.isStatic()) {
            quote(getClassConstructorObj($(gconfig.requireModuleName), $(arkName))[$(arkProp.propKey)])
        } else {
            quote(arkts_object[$(arkProp.propKey)])
        }

        let ctxToken = quote(ctx)
        let toValue = arkProp.typeNode.toJSValue(gconfig, quote(v), ctxToken, isOptional: arkProp.isOptional)
        let setTemplate = genSet(arkProp, propAccess, toValue)
        var propStr = "prop"
        if (!arkProp.isReadOnly()) {
            propStr = "mut " + propStr
        }
        if (arkProp.isStatic()) {
            propStr = "static " + propStr
        }
        let propToken = Token(TokenKind.IDENTIFIER, propStr)

        let fromValue = arkProp.typeNode.fromJSValue(gconfig, propAccess, ctxToken, isOptional: arkProp.isOptional)
        let withOption = getTokenWithoutNL(
            if (arkProp.isOptional) {
                "?" + propType.toString()
            } else {
                propType.toString()
            })
        let getToken = quote(get() {
                checkThreadAndCall<$(withOption)>(getMainContext()) {
                    ctx: JSContext => $(fromValue)
                }
            })
        let template: Tokens = quote(
                public $(propToken) $(propName): $(withOption) {
                    $(getToken)$(setTemplate)
                }
            )
        propertyDeclare.append(template)
    }
}
