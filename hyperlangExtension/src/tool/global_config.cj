// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.collection.*
import std.ast.*
import std.sync.*

public class GlobalConfig {
    public var module: String = ""
    public var moduleFullName: String = ""
    public var cjModuleName: String = ""
    public var outPath: String = ""
    public var moduleMeta: Transform = Transform()
    public var exceptionLog: ArrayList<String> = ArrayList<String>()
    public var isOhos: Bool = false
    public var isHms: Bool = false
    public var requireModuleName: String = ""
    public var requireModulePrefix: ?String = None
    public var requireModulePrefixToken: Token = Token()
    public var is3rd: Bool = false
    public var typeDeclFile: String = ""
    public var gconfigArray: ArrayList<GlobalConfig> = ArrayList<GlobalConfig>()
    public var arkTSMoudleName: String = ""
    public var requiredUnionMembers = ArrayList<(String, TsType, ArrayList<String>)>() 
    public var requiredAnonymousInterfaces = ArrayList<ObjectType>() 
    private var count = AtomicInt64(0)
    private var cacheUnions: HashMap<String, TsType> = HashMap<String, TsType>()
    private var cacheAnonymousInterfaces: HashMap<String, TsType> = HashMap<String, TsType>()

    public init(cjModuleNameArg: String, is3rdArg: Bool,
        typeDeclFileArg!: String = "") {
        cjModuleName = cjModuleNameArg
        is3rd = is3rdArg
        typeDeclFile = typeDeclFileArg
    }

    public func load(_moduleMeta: Transform, _moduleName: String, _moduleFullName: String, _arkTSMoudleName: String,
        _outPath: String): Unit {
        this.module = _moduleName
        this.moduleMeta = _moduleMeta
        this.moduleFullName = _moduleFullName
        this.outPath = _outPath
        this.arkTSMoudleName = _arkTSMoudleName

        if (moduleFullName.contains("@hms")) {
            isHms = true
            requireModulePrefix = "hms"
            requireModuleName = removeTsSuffix(moduleFullName.replace("@hms.", ""))
        } else if (moduleFullName.contains("@ohos")) {
            isOhos = true
            requireModuleName = removeTsSuffix(moduleFullName.replace("@ohos.", ""))
        } else {
            requireModuleName = removeTsSuffix(moduleFullName)
        }
        requireModulePrefixToken = Token(TokenKind.IDENTIFIER, requireModulePrefix ?? "None")

        if (cjModuleName.isEmpty()) {
            cjModuleName = getCjModuleNameForTpc()
        }
    }

    public func isObject(input: String): Bool {
        this.moduleMeta.classes.contains(input) || this.moduleMeta.interfaces.contains(input)
    }

    public func isEnum(input: String): Bool {
        this.moduleMeta.enums.contains(input) || (this.moduleMeta.symbols.get(input)?.contains("enum") ?? false)
    }

    public func getObjectCategory(input: String): String {
        var objCategory = ""
        if (this.moduleMeta.classes.contains(input)) {
            objCategory = "class"
        } else if (this.moduleMeta.interfaces.contains(input)) {
            objCategory = "interface"
        } else {
            throw IllegalArgumentException("Invalid object category")
        }
        return objCategory
    }

    public func isBaseObject(input: String): Bool {
        this.moduleMeta.baseClasses.contains(input) || this.moduleMeta.baseInterfaces.contains(input)
    }

    // TODO: need to find parent in certain namespace and certain module,
    // since different namespace/module may have interface/classes which have same names
    private func findParent(father: String): ?ObjectType {
        for (i in this.gconfigArray) {
            for (j in 0..i.moduleMeta.namespaces.size) {
                let cns: NameSpace = i.moduleMeta.namespaces[j]
                if (cns.interfaces.contains(father)) {
                    return cns.interfaces[father]
                }
                if (cns.classes.contains(father)) {
                    return cns.classes[father]
                }
            }
        }
        return None
    }

    public func collectParentPropertiesAndMethods(i: ObjectType, methods: ArrayList<MethodType>,
        properties: ArrayList<Property>, marked: ArrayList<ObjectType>, first!: Bool = false): Unit {
        for (father in i.parents) {
            match (findParent(father)) {
                case Some(x) => collectParentPropertiesAndMethods(x, methods, properties, marked)
                case None => () // TODO: should add fixme when parent is not found
            }
        }
        if (!marked.contains(i) && !first) {
            methods.add(all: i.methods)
            properties.add(all: i.properties)
            marked.add(i)
        }
    }

    public func collectParentPropertiesAndMethods(): Unit {
        for (i in 0..this.moduleMeta.namespaces.size) {
            let cns: NameSpace = this.moduleMeta.namespaces[i] //csc:current namespace

            if (cns.isEmpty()) {
                continue
            }

            if (cns.interfaces.size > 0) {
                for ((_, i) in cns.interfaces) {
                    let marked = ArrayList<ObjectType>()
                    collectParentPropertiesAndMethods(i, i.parentsMethods, i.parentsProperties, marked, first: true)
                }
            }
            if (cns.classes.size > 0) {
                for ((_, c) in cns.classes) {
                    let marked = ArrayList<ObjectType>()
                    collectParentPropertiesAndMethods(c, c.parentsMethods, c.parentsProperties, marked, first: true)
                }
            }
        }
    }

    public func showConfig(): Unit {
        println("module: ${module}")
        println("output: ${outPath}")
        println("")
    }

    // eg: @ohos.ability.featureAbility.d.ts -> ohos.ability.featureAbility
    // @hms.ai.speechRecognizer.d.ts -> ohos.ai.speechRecognizer
    public func getCjModuleName(): Unit {
        var moduleName = moduleFullName.replace("@", "")
        if (moduleName.contains("hms")) {
            moduleName = moduleName.replace("hms", "ohos")
        }

        removeTsSuffix(moduleName)
    }

    public func getCjModuleNameForTpc(): String {
        let moduleName = moduleFullName.replace("@", "").replace("hms", "").replace("ohos", "")

        "tpc" + removeTsSuffix(moduleName)
    }
    
    // Collect all required union member counts by scanning all types in the module
    public func collectRequiredUnionMemberCounts(): Unit {
        for (i in 0..this.moduleMeta.namespaces.size) {
            let cns: NameSpace = this.moduleMeta.namespaces[i]
            if (cns.isEmpty()) {
                continue
            }
            
            // Scan properties in classes
            for ((_, c) in cns.classes) {
                let result = HashSet<String>()
                c.typeParameters |> map {it => it.name.replace(" ", "")} |> forEach {item => 
                    result.add(item)
                }
                for (p in c.properties) {
                    if (let Some(v) <- scanTypeForAnonymousInterface(p.typeNode)) {
                        p.typeNode = v
                    }
                    if (let Some(v) <- scanTypeForUnions(p.typeNode, result)) {
                        p.typeNode = v
                    }
                }
                // Scan methods in classes
                for (m in c.methods) {
                    for (param in m.parameters) {
                        if (let Some(v) <- scanTypeForAnonymousInterface(param.ty)) {
                            param.ty = v
                        }
                    }
                    if (let Some(v) <- scanTypeForAnonymousInterface(m.returnTypeNode)) {
                        m.returnTypeNode = v
                    }
                    m.typeParameters |> map {it => it.name.replace(" ", "")} |> forEach {item => 
                        result.add(item)
                    }
                    if (let Some(v) <- scanTypeForUnions(m.returnTypeNode, result)) {
                        m.returnTypeNode = v
                    }
                }
            }
            
            // Scan properties in interfaces
            for ((_, iface) in cns.interfaces) {
                let result = HashSet<String>()
                iface.typeParameters |> map {it => it.name.replace(" ", "")} |> forEach {item => 
                    result.add(item)
                }
                for (p in iface.properties) {
                    if (let Some(v) <- scanTypeForAnonymousInterface(p.typeNode)) {
                        p.typeNode = v
                    }
                    if (let Some(v) <- scanTypeForUnions(p.typeNode, result)) {
                        p.typeNode = v
                    }
                }
                // Scan methods in interfaces
                for (m in iface.methods) {
                    for (param in m.parameters) {
                        if (let Some(v) <- scanTypeForAnonymousInterface(param.ty)) {
                            param.ty = v
                        }
                    }
                    if (let Some(v) <- scanTypeForAnonymousInterface(m.returnTypeNode)) {
                        m.returnTypeNode = v
                    }
                    m.typeParameters |> map {it => it.name.replace(" ", "")} |> forEach {item => 
                        result.add(item)
                    }
                    if (let Some(v) <- scanTypeForUnions(m.returnTypeNode, result)) {
                        m.returnTypeNode = v
                    }   
                }
            }
            
            // Scan global methods
            for (m in cns.methods) {
                for (param in m.parameters) {
                    if (let Some(v) <- scanTypeForAnonymousInterface(param.ty)) {
                            param.ty = v
                    }
                }
                if (let Some(v) <- scanTypeForAnonymousInterface(m.returnTypeNode)) {
                        m.returnTypeNode = v
                }

                /* Consider the following example, where generic parameters need to be collected.

                    declare function testGeneric<T>(t:T): T | string
                */
                let result = HashSet<String>()
                m.typeParameters |> map {it => it.name.replace(" ", "")} |> forEach {item => 
                    result.add(item)
                }
                if (let Some(v) <- scanTypeForUnions(m.returnTypeNode,  result)) {
                        m.returnTypeNode = v
                } 
            }
        }
    }
    
    // Recursively scan a type for union types and record their member counts
    private func scanTypeForUnions(ty: TsType, genericParameters: HashSet<String>): Option<TsType> {
        match (ty) {
            case TsUnion(elems) =>
                let elements = elems.map {item => 
                    if (let Some(v) <- scanTypeForUnions(item, genericParameters)) {
                        v
                    } else {
                        item
                    }
                } 
                let nonNullTypes = ArrayList<TsType>()
                for (e in elements) {
                    match (e) {
                        case TsNull | TsUndefined => ()
                        case _ => nonNullTypes.add(e)
                    }
                }
                let result = ArrayList<String>()
                for (e in elements) {
                    if (genericParameters.contains(e.toString())) {
                        result.add(e.toString())
                    }
                }
                // Only count primitive unions with 2+ members
                if (nonNullTypes.size > 1) {
                    let tempName = ty.toString().replace(" ", "")
                    if (cacheUnions.contains(tempName)) {
                        return Some(cacheUnions[tempName])
                    } else {
                        let paramName = "AutoGenType" + count.load().toString()
                        requiredUnionMembers.add((paramName, ty, result))
                        count.fetchAdd(1)
                        cacheUnions[tempName] = TsTypeRef(paramName)
                        let temp = result |> map {item => 
                            let result = TypeParameter()
                            result.name = item
                            result
                        } |> collectArrayList
                        return Some(TsTypeRef(paramName + getTypeParametersToken(temp).toString()))
                    }
                }
               return TsUnion(elements) 
            case TsArray(elemTy) => 
                let elementType = if (let Some(v) <- scanTypeForUnions(elemTy, genericParameters)) {
                    v
                } else {
                    elemTy
                }
                return TsArray(elementType)
            case TsOptional(elemTy) => scanTypeForUnions(elemTy, genericParameters)
            case TsTypeRef(temp, arr) =>
                let elements = arr.map {item => 
                    if (let Some(v) <- scanTypeForUnions(item, genericParameters)) {
                        v
                    } else {
                        item
                    }
                }
                return TsTypeRef(temp, elements) 
            case TsTuple(elems) =>
                let elements = elems.map {item => 
                    if (let Some(v) <- scanTypeForUnions(item, genericParameters)) {
                        v
                    } else {
                        item
                    }
                } 
                return TsTuple(elements)
            case TsFunction(ret, params, temp) =>
                let returnType = if (let Some(v) <- scanTypeForUnions(ret, genericParameters)) {
                        v
                    } else {
                        ret
                    }
                let elements = params.map {item => 
                    if (let Some(v) <- scanTypeForUnions(item.ty, genericParameters)) {
                        item.ty = v
                        item
                    } else {
                        item
                    }
                }
                return TsFunction(returnType, elements, temp)
            case _ => ()
        }
        return None
    }

    private func scanTypeForAnonymousInterface(ty: TsType): Option<TsType> {
        match (ty) {
            case TsUnion(elems) =>
                let elements = elems.map {item => 
                    if (let Some(v) <- scanTypeForAnonymousInterface(item)) {
                        v
                    } else {
                        item
                    }
                } 
                return TsUnion(elements)
            case TsArray(elemTy) => 
                let elementType = if (let Some(v) <- scanTypeForAnonymousInterface(elemTy)) {
                    v
                } else {
                    elemTy
                }
                return TsArray(elementType)
            case TsOptional(elemTy) => scanTypeForAnonymousInterface(elemTy)
            case TsTypeRef(temp, arr) =>
                let elements = arr.map {item => 
                    if (let Some(v) <- scanTypeForAnonymousInterface(item)) {
                        v
                    } else {
                        item
                    }
                }
                return TsTypeRef(temp, elements) 
            case TsTuple(elems) =>
                let elements = elems.map {item => 
                    if (let Some(v) <- scanTypeForAnonymousInterface(item)) {
                        v
                    } else {
                        item
                    }
                } 
                return TsTuple(elements)
            case TsFunction(ret, params, temp) =>
                let returnType = if (let Some(v) <- scanTypeForAnonymousInterface(ret)) {
                        v
                    } else {
                        ret
                    }
                let elements = params.map {item => 
                    if (let Some(v) <- scanTypeForAnonymousInterface(item.ty)) {
                        item.ty = v
                        item
                    } else {
                        item
                    }
                }
                return TsFunction(returnType, elements, temp)
            case TsTypeLiteral(members) =>
                let propertiesNew = ArrayList<Property>()
                let boxed = Box<ArrayList<Property>>(propertiesNew)
                members.map {item:TsMember => 
                    match (item) {
                        case TsProperty(name, ty, optional) => 
                            let result = Property()
                            result.propKey = name
                            result.propType = ty.toString()
                            result.cjKey = name
                            result.isOptional = optional
                            result.typeNode = ty
                            boxed.value.add(result)
                            ()
                        case _ =>
                            ()
                    }
            
                }
                let tempName = ty.toString().replace(" ", "")
                if (cacheAnonymousInterfaces.contains(tempName)) {
                    return Some(cacheUnions[tempName])
                } else {
                    var objectType = ObjectType()
                    objectType.properties = propertiesNew
                    let paramName = "AutoGenType" + count.load().toString() 
                    objectType.name = paramName
                    requiredAnonymousInterfaces.add(objectType)
                    count.fetchAdd(1)
                    return Some(TsTypeRef(paramName))
                }
                
            case _ => ()
        }
        return ty
    }
}
