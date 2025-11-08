// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.env.*
import std.fs.*
import std.collection.*
import std.convert.*
import stdx.serialization.serialization.*
import cjpm.toml.*
import cjpm.toml.Decoder as DecoderToml

public let REPO_CONFIG_FILE_NAME = "cjpm-config.toml"

public class RepositoryInfo <: Serializable<RepositoryInfo> {
    public RepositoryInfo(
        public var path!: String = "",
        public var registry!: String = "",
        public var token!: String = ""
    ) {}

    public func serialize(): DataModel {
        let result = DataModelStruct()
        result.add(field<String>("path", this.path))
        result.add(field<String>("registry", this.registry))
        result.add(field<String>("token", this.token))
        return result
    }

    public static func deserialize(dm: DataModel): RepositoryInfo {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = RepositoryInfo()
        result.path = String.deserialize((dms.get("path") as DataModelString) ?? DataModelString(""))
        result.registry = String.deserialize((dms.get("registry") as DataModelString) ?? DataModelString(""))
        result.token = String.deserialize((dms.get("token") as DataModelString) ?? DataModelString(""))
        return result
    }
}

public class SettingsConfig <: Serializable<SettingsConfig> {
    public var repository: HashMap<String, RepositoryInfo> = HashMap<String, RepositoryInfo>()
    public var isValid: Bool = true

    public init() {
        this.repository.add("local", RepositoryInfo())
        this.repository["local"].path = Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").toString()
    }

    public func serialize(): DataModel {
        let result = DataModelStruct()
        result.add(field<HashMap<String, RepositoryInfo>>("repository", repository))
        return result
    }

    public static func deserialize(dm: DataModel): SettingsConfig {
        let dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = SettingsConfig()
        result.repository = HashMap<String, RepositoryInfo>.deserialize((dms.get("repository") as DataModelStruct) ?? DataModelStruct())

        if (!result.repository.contains("local")) {
            result.repository.add("local", RepositoryInfo())
            result.repository["local"].path = Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").toString()
        }

        return result
    }
}

public func readConfigToml(file: Path): SettingsConfig {
    var res = SettingsConfig()
    try {
        let fileInfo = File(file, Read)
        let de = DecoderToml(fileInfo)
        let to = de.decode()
        res = SettingsConfig.deserialize(DataModel.fromToml(to))
        fileInfo.close()
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: load file '${file}' failed.")
        res.isValid = false
        return res
    }

    // Invaild config when cannot get [repository.home] data)
    if (res.repository.isEmpty() || !res.repository.contains("home") || res.repository["home"].registry.isEmpty()) {
        println("Error: load config data '[repository.home]'  from '${file}' failed.")
        res.isValid = false
        return res
    }

    if (res.repository.contains("local") && !res.repository["local"].path.isEmpty()) {
        // Convert the path to an absolute path when path is relative path
        res.repository["local"].path = getPath(file.parent.toString(), res.repository["local"].path)
    }

    if (res.repository.contains("local") && res.repository["local"].path.isEmpty()) {
        // when path of [repository.local] is empty, set $HOME/.cjpm or %USERPROFILE%/.cjpm as repository local path
        res.repository["local"].path = Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").toString()
    }

    return res
}

public func loadConfigInfo(): SettingsConfig {
    // Read the configuration file based on priority.
    if (fileExists(Path(DIR_CURRENT).join(REPO_CONFIG_FILE_NAME))) {
        return readConfigToml(Path(DIR_CURRENT).join(REPO_CONFIG_FILE_NAME))
    }

    if (fileExists(Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").join(REPO_CONFIG_FILE_NAME))) {
        return readConfigToml(Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").join(REPO_CONFIG_FILE_NAME))
    }

    var res = SettingsConfig()
    let cj_home: String = getVariable("CANGJIE_HOME") ?? return res
    if (fileExists(Path(cj_home).join("tools").join("config").join(REPO_CONFIG_FILE_NAME))) {
        res = readConfigToml(Path(cj_home).join("tools").join("config").join(REPO_CONFIG_FILE_NAME))
        let globalCjpmConfig = getVariable("CJPM_CONFIG") ?? return res
        println("Warning: the functionality of 'CJPM_CONFIG' will be deprecated, " +
            "please unset 'CJPM_CONFIG' and use cjpm-config.toml to set the repository local path")
        res.repository["local"].path = globalCjpmConfig
        return res
    }

    let globalCjpmConfig = getVariable("CJPM_CONFIG") ?? return res
    println("Warning: the functionality of 'CJPM_CONFIG' will be deprecated, " +
            "please unset 'CJPM_CONFIG' and use cjpm-config.toml to set the repository local path")
    res.repository["local"].path = globalCjpmConfig

    return res
}

func versionCompare(v1: String, v2: String): Bool {
    let vArr1: Array<String> = v1.split(".")
    let vArr2: Array<String> = v2.split(".")
    let vArrList1: ArrayList<Int64> = ArrayList<Int64>()
    let vArrList2: ArrayList<Int64> = ArrayList<Int64>()
    for (index in 0..3) {
        vArrList1.add(Int64.parse(vArr1[index]))
        vArrList2.add(Int64.parse(vArr2[index]))
    }
    if (vArrList2[0] > vArrList1[0] || (vArrList2[0] == vArrList1[0] && vArrList2[1] > vArrList1[1]) ||
        (vArrList2[0] == vArrList1[0] && vArrList2[1] == vArrList1[1] && vArrList2[2] > vArrList1[2])) {
        return true
    }
    return false
}

public func verifyDependencyField(dep: DepInfo, name: String, path: String): Bool {
    let isPathDep = dep.path.isSome()
    let isGitDep = dep.git.isSome()
    let isRepoDep = dep.version.isSome()

    if (!isPathDep && !isGitDep && !isRepoDep) {
        eprintln("Error: one of 'git' or 'path' or 'version' fields must exist " +
            "in dependency ${name} in ${Path(path).join(CONFIG_FILE_NAME).toString()}")
        return false
    }
    if ((isPathDep && isGitDep) || (isPathDep && isRepoDep) || (isGitDep && isRepoDep)) {
        eprintln("Error: only one of 'git' or 'path' or 'version' fields can exist " +
            "in dependency ${name} in ${Path(path).join(CONFIG_FILE_NAME).toString()}")
        return false
    }

    if (let Some(version) <- dep.version) {
        return centralVersionCheck(name, version, Path(path).join(CONFIG_FILE_NAME).toString())
    }

    return true
}