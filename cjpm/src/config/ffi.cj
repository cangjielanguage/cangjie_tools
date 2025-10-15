// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import stdx.serialization.serialization.*

class CLibrary <: Serializable<CLibrary> {
    public var path: String = ""

    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): CLibrary {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = CLibrary()
        result.path = String.deserialize((dms.get("path") as DataModelString) ?? DataModelString(""))
        return result
    }
}

public class ForeignInterfaces <: Serializable<ForeignInterfaces> {
    public var cLibs: HashMap<String, String> = HashMap<String, String>()

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): ForeignInterfaces {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = ForeignInterfaces()
        var cLibraries = HashMap<String, CLibrary>.deserialize((dms.get("c") as DataModelStruct) ?? DataModelStruct())
        for ((cLibName, cLibrary) in cLibraries) {
            result.cLibs[cLibName] = cLibrary.path
        }

        return result
    }

    public func combineForeigns(ffi: ForeignInterfaces): Unit {
        for ((cffiKey, cffiValue) in ffi.cLibs) {
            if (this.cLibs.contains(cffiKey)) {
                eprintln("Warning: c-library '${cffiKey}' in workspace is already imported by members, " +
                    "skip this configuration in workspace")
            } else {
                this.cLibs.add(cffiKey, cffiValue)
            }
        }
    }
}

func verifyCLibConfig(cLibs: HashMap<String, String>, path: String, jsonPath: String, targetString!: String = ""): Bool {
    var flag = true
    for ((cLibName, cLibPath) in cLibs) {
        if (!cLibNameCheck(cLibName, jsonPath)) {
            flag = false
        }

        // Path normalization in foreignRequires.
        cLibs[cLibName] = getPath(path, cLibPath)
        if (!safeCheck(cLibs[cLibName], "'${targetString}ffi.c' field at ${jsonPath}")) {
            flag = false
        }

        // File exist Check.
        if (!isCLibExist(cLibName, cLibs[cLibName], "'${targetString}ffi.c' field at ${jsonPath}")) {
            flag = false
        }
    }
    return flag
}
