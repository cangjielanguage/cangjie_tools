// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package hle.tool

import std.collection.*
import std.ast.*

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
}
