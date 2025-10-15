// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import stdx.serialization.serialization.*
import cjpm.util.WithDefault

const FILE_VERSION = "cjc-version"
const PACKAGE_NAME = "name"
const DESCRIPTION = "description"
const VERSION = "version"
const COMPILE_OPTION = "compile-option"
const OVERRIDE_COMPILE_OPTION = "override-compile-option"
const LINK_OPTION = "link-option"
const OUTPUT_TYPE = "output-type"
const SRC_DIR = "src-dir"
const TARGET_DIR = "target-dir"
const PACKAGE_CONFIGURATION = "package-configuration"

public class Package <: Serializable<Package> {
    public var cjcVersion: String = ""
    public var name: String = ""
    public var description: String = "nothing here"
    public var version: String = "1.0.0"
    public var outputType: OutputType = Unknown("")
    public var compileOption: String = ""
    public var overrideCompileOption: String = ""
    public var linkOption: String = ""
    public var nativeCompileOption: String = ""
    public var nativeLinkOption: String = ""
    public var path: String = ""
    public var packageConfiguration: HashMap<String, PackageConfiguration> = HashMap<String, PackageConfiguration>()
    public var targetDir: String = ""
    public var srcDir: WithDefault<String> = WithDefault(DIR_SRC)
    public var validPkgMap: Array<PackageInfo> = Array<PackageInfo>()
    public var actualSources: CMPSource = CMPSource.empty
    public var sourceSetTree: SourceSetsConfiguration = SourceSetsConfiguration()

    public func serialize(): DataModel {
        return serializePackage(this)
    }

    public static func deserialize(dm: DataModel): Package {
        return deserializePackage(dm)
    }
}

func serializePackage(pkg: Package): DataModel {
    let dms = DataModelStruct()
        .add(field<String>(FILE_VERSION, pkg.cjcVersion))
        .add(field<String>(PACKAGE_NAME, pkg.name))
        .add(field<String>(DESCRIPTION, pkg.description))
        .add(field<String>(VERSION, pkg.version))
        .add(field<String>(TARGET_DIR, pkg.targetDir))
        .add(field<OutputType>(OUTPUT_TYPE, pkg.outputType))
        .add(field<String>(COMPILE_OPTION, pkg.compileOption))
        .add(field<String>(OVERRIDE_COMPILE_OPTION, pkg.overrideCompileOption))
        .add(field<String>(LINK_OPTION, pkg.linkOption))
        .add(field<HashMap<String, PackageConfiguration>>(PACKAGE_CONFIGURATION, pkg.packageConfiguration))
    if (pkg.srcDir.isSet) {
        dms.add(field<String>(SRC_DIR, pkg.srcDir.value))
    } else {
        dms
    }
}

func deserializePackage(dm: DataModel): Package {
    var dms = match (dm) {
        case data: DataModelStruct => data
        case _ => throw Exception("this data is not DataModelStruct")
    }
    var result = Package()
    result.cjcVersion = String.deserialize((dms.get(FILE_VERSION) as DataModelString) ?? DataModelString(""))
    result.name = String.deserialize((dms.get(PACKAGE_NAME) as DataModelString) ?? DataModelString(""))
    result.description = String.deserialize((dms.get(DESCRIPTION) as DataModelString) ?? DataModelString(""))
    result.version = String.deserialize((dms.get(VERSION) as DataModelString) ?? DataModelString(""))
    result.targetDir = String.deserialize((dms.get(TARGET_DIR) as DataModelString) ?? DataModelString(""))
    if (let Some(dmstr) <- (dms.get(SRC_DIR) as DataModelString) && !dmstr.getValue().isEmpty()) {
        result.srcDir.modify(dmstr.getValue())
    }
    result.outputType = OutputType.deserialize((dms.get(OUTPUT_TYPE) as DataModelString) ?? DataModelString(""))
    result.compileOption = String.deserialize((dms.get(COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
    result.overrideCompileOption = String.deserialize(
        (dms.get(OVERRIDE_COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
    result.linkOption = String.deserialize((dms.get(LINK_OPTION) as DataModelString) ?? DataModelString(""))
    result.packageConfiguration = HashMap<String, PackageConfiguration>.deserialize(
        (dms.get(PACKAGE_CONFIGURATION) as DataModelStruct) ?? DataModelStruct())
    return result
}

public class PackageConfiguration <: Serializable<PackageConfiguration> {
    public var outputType: OutputType = Unknown("")
    public var compileOption: String = ""

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): PackageConfiguration {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = PackageConfiguration()
        result.outputType = OutputType.deserialize((dms.get(OUTPUT_TYPE) as DataModelString) ?? DataModelString(""))
        result.compileOption = String.deserialize((dms.get(COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
        return result
    }
}

public func verifyPackageField(cfg: TomlInfo, path: String): Bool {
    let configFilePath: String = Path(path).join(CONFIG_FILE_NAME).toString()
    cfg.pkg.path = path

    if (!verifyPackageConfig(cfg.pkg.packageConfiguration, configFilePath)) {
        return false
    }

    if (!state.isExperimental) {
        if (!srcDirCheck(cfg.pkg.srcDir.value, path, configFilePath)) {
            return false
        }
        cfg.pkg.srcDir.modify(getPath(path, cfg.pkg.srcDir.value))
    }

    let checkList: Array<((String, String) -> Bool, ?String)> = [
        (nameCheck, cfg.pkg.name),
        (cjcVersionCheck, cfg.pkg.cjcVersion),
        (versionCheck, cfg.pkg.version),
        (safeCheck, cfg.pkg.compileOption),
        (safeCheck, cfg.pkg.overrideCompileOption),
        (outputTypeCheck, cfg.pkg.outputType.toString()),
        (safeCheck, cfg.pkg.targetDir),
        (verifyLinkOption, cfg.pkg.linkOption)
    ]
    if (!verifyAll(configFilePath, checkList)) {
        return false
    }

    if (!verifyEnvConfig(cfg.profile.run.envConfig, "run", configFilePath)) {
        return false
    }

    return true
}

func verifyPackageConfig(packageConfiguration: HashMap<String, PackageConfiguration>, path: String): Bool {
    for ((_, v) in packageConfiguration) {
        if (!safeCheck(v.compileOption, path)) {
            return false
        }
    }
    return true
}
