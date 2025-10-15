// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import stdx.serialization.serialization.*

public class ImportInfo <: Serializable<ImportInfo> {
    public var fileImported: String = ""

    public func serialize(): DataModel {
        let result = DataModelStruct()
        result.add(field<String>("file", this.fileImported))
        return result
    }

    public static func deserialize(dm: DataModel): ImportInfo {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = ImportInfo()
        result.fileImported = String.deserialize((dms.get("file") as DataModelString) ?? DataModelString(""))
        return result
    }
}

public struct FullName <: Serializable<FullName> {
    private FullName(
        public let packageName: String,
        public let isStdlibPackage: Bool,
        public let orPkgName: String,
        public let importsList: Array<ImportInfo>
    ) {}

    public func serialize(): DataModel {
        let result = DataModelStruct()
        result.add(field<String>("orPackage", this.orPkgName))
        result.add(field<String>("package", this.packageName))
        result.add(field<Bool>("isStd", this.isStdlibPackage))
        result.add(field<Array<ImportInfo>>("imports", this.importsList))
        return result
    }

    public static func deserialize(dm: DataModel): FullName {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        let packageName = String.deserialize(dms.get("package"))
        let isStdlibPackage = Bool.deserialize(dms.get("isStd"))
        let orPkgName = String.deserialize((dms.get("orPackage") as DataModelString) ?? DataModelString(""))
        let importsList = Array<ImportInfo>.deserialize((dms.get("imports") as DataModelSeq) ?? DataModelSeq())

        FullName(
            packageName,
            isStdlibPackage,
            orPkgName,
            importsList
        )
    }
}

public class RequiresPackages <: Serializable<RequiresPackages> {
    public var packageName: String = ""
    public var accessLevel: String = ""
    public var isMacroPackage: Bool = false
    public var mockUsed: Bool = false
    public var testUsed: Bool = false
    public var dependencies: ArrayList<FullName> = ArrayList<FullName>()
    public var stdDependencies: ArrayList<FullName> = ArrayList<FullName>()
    public var timeStamp: Int64 = 0

    public func serialize(): DataModel {
        let result = DataModelStruct()
        result.add(field<String>("package", this.packageName))
        result.add(field<Bool>("isMacro", this.isMacroPackage))
        result.add(field<String>("accessLevel", this.accessLevel))
        result.add(field<Int64>("timeStamp", this.timeStamp))
        result.add(field<ArrayList<FullName>>(DEPENDENCIES, this.dependencies))
        result.add(field<ArrayList<FullName>>("std-dependencies", this.stdDependencies))
        result.add(field<Bool>("testUsed", this.testUsed))
        result.add(field<Bool>("mockUsed", this.mockUsed))
        return result
    }

    public static func deserialize(dm: DataModel): RequiresPackages {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = RequiresPackages()
        result.packageName = String.deserialize(dms.get("package"))
        result.isMacroPackage = Bool.deserialize(dms.get("isMacro"))
        result.accessLevel = String.deserialize((dms.get("accessLevel") as DataModelString) ?? DataModelString("public"))
        result.timeStamp = Int64.deserialize((dms.get("timeStamp") as DataModelInt) ?? DataModelInt(0))
        result.testUsed = Bool.deserialize((dms.get("testUsed") as DataModelBool) ?? DataModelBool(false))
        result.mockUsed = Bool.deserialize((dms.get("mockUsed") as DataModelBool) ?? DataModelBool(false))
        let allDependencies = ArrayList<FullName>.deserialize(dms.get(DEPENDENCIES))
        for (dependency in allDependencies) {
            if (!result.testUsed && isTestPackage(dependency)) {
                result.testUsed = true
            }
            if (!result.mockUsed && isMockPackage(dependency)) {
                result.mockUsed = true
            }
            if (result.testUsed && result.mockUsed) {
                break
            }
        }
        result.dependencies = allDependencies |> filter {dependency: FullName => !dependency.isStdlibPackage} |>
            collectArrayList<FullName>
        result.stdDependencies = allDependencies |> filter {dependency: FullName => dependency.isStdlibPackage} |>
            collectArrayList<FullName>
        result.stdDependencies.add(all: ArrayList<FullName>.deserialize(
            (dms.get("std-dependencies") as DataModelSeq) ?? DataModelSeq()))
        return result
    }
}

public class RequiresPackagesInfo <: Serializable<RequiresPackagesInfo> {
    public var isMacroPackage: Bool = false
    public var requires: HashSet<String> = HashSet<String>()
    public var stdRequires: HashSet<String> = HashSet<String>()
    public var cjPath: String = ""
    public var libPath: String = ""
    public var libName: String = ""
    public var isDyLib: Bool = false

    public init() {}

    public init(isMacroPackage: Bool, cjPath: String, libPath: String) {
        this.isMacroPackage = isMacroPackage
        this.cjPath = cjPath
        this.libPath = libPath
    }

    public func serialize(): DataModel {
        return DataModelStruct()
            .add(field<Bool>("isMacroPackage", isMacroPackage))
            .add(field<HashSet<String>>("requires", requires))
            .add(field<HashSet<String>>("std-requires", stdRequires))
            .add(field<String>("cjPath", cjPath))
            .add(field<String>("libPath", libPath))
            .add(field<String>("libName", libName))
    }

    public static func deserialize(dm: DataModel): RequiresPackagesInfo {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = RequiresPackagesInfo()
        result.isMacroPackage = Bool.deserialize(dms.get("isMacroPackage"))
        result.requires = HashSet<String>.deserialize(dms.get("requires"))
        result.stdRequires = HashSet<String>.deserialize(dms.get("std-requires"))
        result.cjPath = String.deserialize(dms.get("cjPath"))
        result.libPath = String.deserialize(dms.get("libPath"))
        result.libName = String.deserialize(dms.get("libName"))
        return result
    }
}

public func makeCangjieStaticlibName(file: String): String {
    return "lib" + file + ".a"
}

@When[os == "Linux"]
public func makeLtoName(file: String): String {
    return file + ".bc"
}

public enum MockSupport <: Equatable<MockSupport> & ToString {
    On | Off | RuntimeError

    public override func toString(): String {
        match (this) {
            case On => "on"
            case Off => "off"
            case RuntimeError => "runtime-error"
        }
    }

    public operator override func ==(other: MockSupport): Bool {
        match ((this, other)) {
            case (On, On) => true
            case (Off, Off) => true
            case (RuntimeError, RuntimeError) => true
            case _ => false
        }
    }

    public operator override func !=(other: MockSupport): Bool {
        !(this == other)
    }

    public static func fromString(str: String): MockSupport {
        match (str) {
            case "on" => MockSupport.On
            case "off" => MockSupport.Off
            case "runtime-error" => MockSupport.RuntimeError
            case _ => throw IllegalStateException("Unsupported kind of mock support")
        }
    }
}

func isMockPackage(dependency: FullName) {
    return dependency.isStdlibPackage && (dependency.packageName == "std.unittest.mock" ||
        dependency.packageName == "std.unittest.mock.mockmacro")
}

func isTestPackage(dependency: FullName) {
    return dependency.isStdlibPackage && (dependency.packageName == "std.unittest" ||
        dependency.packageName == "std.unittest.testmacro")
}
