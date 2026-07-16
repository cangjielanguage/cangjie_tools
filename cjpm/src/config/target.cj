// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import stdx.serialization.serialization.*

let BIN_DEPENDENCIES: String = "bin-dependencies"
let FFI: String = "ffi"
const PATH_OPTION = "path-option"
const PACKAGE_OPTION = "package-option"
public const COMPILE_TARGET_MACROS: String = "compile-macros-for-target"

public class BinDependencies <: Serializable<BinDependencies> {
    public var pathOption: ArrayList<String> = ArrayList<String>()
    public var packageOption: HashMap<String, String> = HashMap<String, String>()

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): BinDependencies {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = BinDependencies()
        result.pathOption = ArrayList<String>.deserialize((dms.get(PATH_OPTION) as DataModelSeq) ?? DataModelSeq())
        result.packageOption = HashMap<String, String>.deserialize(
            (dms.get(PACKAGE_OPTION) as DataModelStruct) ?? DataModelStruct())
        return result
    }
}

public class TargetInfo <: Serializable<TargetInfo> {
    public var targetName: String = ""
    public var compileOption: String = ""
    public var overrideCompileOption: String = ""
    public var linkOption: String = ""
    public var dependencies: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var testDependencies: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var scriptDependencies: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var replaceDeps: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var binDependencies: BinDependencies = BinDependencies()
    public var someTargetMacros: ArrayList<String> = ArrayList<String>() // used by cross compile
    public var allTargetMacros: String = "" // used by cross compile
    public var ffi: ForeignInterfaces = ForeignInterfaces()

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): TargetInfo {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = TargetInfo()
        result.compileOption = String.deserialize((dms.get(COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
        result.overrideCompileOption = String.deserialize(
            (dms.get(OVERRIDE_COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
        result.linkOption = String.deserialize((dms.get(LINK_OPTION) as DataModelString) ?? DataModelString(""))
        result.dependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.testDependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(TEST_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.scriptDependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(SCRIPT_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.replaceDeps = HashMap<String, DepInfo>.deserialize(
            (dms.get(REPLACE_DEP) as DataModelStruct) ?? DataModelStruct())
        result.binDependencies = BinDependencies.deserialize(
            (dms.get(BIN_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.ffi = ForeignInterfaces.deserialize((dms.get(FFI) as DataModelStruct) ?? DataModelStruct())
        return result
    }

    public func replaceEnvVars(): Unit {
        this.compileOption = replaceEnvVars(this.compileOption)
        this.overrideCompileOption = replaceEnvVars(this.overrideCompileOption)
        this.linkOption = replaceEnvVars(this.linkOption)
        for ((_, dep) in this.dependencies) {
            if (dep.path.isSome()) {
                dep.path = replaceEnvVars(dep.path.getOrThrow())
            }
        }
        for ((_, dep) in this.testDependencies) {
            if (dep.path.isSome()) {
                dep.path = replaceEnvVars(dep.path.getOrThrow())
            }
        }
        for ((_, dep) in this.scriptDependencies) {
            if (dep.path.isSome()) {
                dep.path = replaceEnvVars(dep.path.getOrThrow())
            }
        }
        for ((_, dep) in this.replaceDeps) {
            if (dep.path.isSome()) {
                dep.path = replaceEnvVars(dep.path.getOrThrow())
            }
        }
        let pathList = ArrayList<String>()
        for (k in this.binDependencies.pathOption) {
            pathList.add(replaceEnvVars(k))
        }
        this.binDependencies.pathOption = pathList

        for ((k, v) in this.binDependencies.packageOption) {
            this.binDependencies.packageOption[k] = replaceEnvVars(v)
        }
        for ((k, v) in this.ffi.cLibs) {
            this.ffi.cLibs[k] = replaceEnvVars(v)
        }
    }

    public func getCheckEnvVars(): ArrayList<String> {
        var res = ArrayList<String>()
        res.add(this.compileOption)
        res.add(this.overrideCompileOption)
        res.add(this.linkOption)
        for ((_, v) in this.dependencies) {
            if (v.path.isSome()) {
                res.add(v.path.getOrThrow())
            }
        }
        for ((_, v) in this.testDependencies) {
            if (v.path.isSome()) {
                res.add(v.path.getOrThrow())
            }
        }
        for ((_, v) in this.scriptDependencies) {
            if (v.path.isSome()) {
                res.add(v.path.getOrThrow())
            }
        }
        for ((_, v) in this.replaceDeps) {
            if (v.path.isSome()) {
                res.add(v.path.getOrThrow())
            }
        }
        res.add(all: getChecker(this.binDependencies.packageOption))
        res.add(all: this.binDependencies.pathOption)
        return res
    }
}

public class TargetConfig <: Serializable<TargetConfig> {
    public var targetName: String = ""
    public var commonTargetInfo: TargetInfo = TargetInfo()
    public var debugTargetInfo: TargetInfo = TargetInfo()
    public var releaseTargetInfo: TargetInfo = TargetInfo()

    // Serialize of toml is useless.
    public func serialize(): DataModel {
        return DataModelStruct()
    }

    public static func deserialize(dm: DataModel): TargetConfig {
        var dms = match (dm) {
            case data: DataModelStruct => data
            case _ => throw Exception("this data is not DataModelStruct")
        }
        var result = TargetConfig()
        match (dms.get(COMPILE_TARGET_MACROS)) {
            case data: DataModelSeq => result.commonTargetInfo.someTargetMacros = ArrayList<String>.deserialize(data)
            case str: DataModelString => result.commonTargetInfo.allTargetMacros = str.getValue()
            case _ => ()
        }
        result.commonTargetInfo.compileOption = String.deserialize(
            (dms.get(COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
        result.commonTargetInfo.overrideCompileOption = String.deserialize(
            (dms.get(OVERRIDE_COMPILE_OPTION) as DataModelString) ?? DataModelString(""))
        result.commonTargetInfo.linkOption = String.deserialize(
            (dms.get(LINK_OPTION) as DataModelString) ?? DataModelString(""))
        result.commonTargetInfo.dependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.commonTargetInfo.testDependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(TEST_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.commonTargetInfo.scriptDependencies = HashMap<String, DepInfo>.deserialize(
            (dms.get(SCRIPT_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.commonTargetInfo.replaceDeps = HashMap<String, DepInfo>.deserialize(
            (dms.get(REPLACE_DEP) as DataModelStruct) ?? DataModelStruct())
        result.commonTargetInfo.binDependencies = BinDependencies.deserialize(
            (dms.get(BIN_DEPENDENCIES) as DataModelStruct) ?? DataModelStruct())
        result.commonTargetInfo.ffi = ForeignInterfaces.deserialize((dms.get(FFI) as DataModelStruct) ??
            DataModelStruct())
        result.debugTargetInfo = TargetInfo.deserialize((dms.get("debug") as DataModelStruct) ?? DataModelStruct())
        result.releaseTargetInfo = TargetInfo.deserialize((dms.get("release") as DataModelStruct) ?? DataModelStruct())
        return result
    }

    public func replaceEnvVars(): Unit {
        this.commonTargetInfo.replaceEnvVars()
        this.debugTargetInfo.replaceEnvVars()
        this.releaseTargetInfo.replaceEnvVars()
    }

    public func getCheckEnvVars(): ArrayList<String> {
        var res = ArrayList<String>()
        res.add(all: this.commonTargetInfo.getCheckEnvVars())
        res.add(all: this.debugTargetInfo.getCheckEnvVars())
        res.add(all: this.releaseTargetInfo.getCheckEnvVars())
        return res
    }
}

public func replaceTargetInfo(path: String, oriInfo: TargetInfo, newInfo: TargetInfo, targetName: String, isTest: Bool): Bool {
    oriInfo.compileOption = combineOptionString(oriInfo.compileOption, newInfo.compileOption)
    oriInfo.overrideCompileOption = combineOptionString(oriInfo.overrideCompileOption, newInfo.overrideCompileOption)
    oriInfo.linkOption = combineOptionString(oriInfo.linkOption, newInfo.linkOption)
    if (!combineDependencies(path, oriInfo.dependencies, newInfo.dependencies, targetName)) {
        return false
    }
    if (isTest && !combineDependencies(path, oriInfo.testDependencies, newInfo.testDependencies, targetName)) {
        return false
    }
    if (!combineDependencies(path, oriInfo.scriptDependencies, newInfo.scriptDependencies, targetName)) {
        return false
    }
    if (!combineDependencies(path, oriInfo.replaceDeps, newInfo.replaceDeps, targetName)) {
        return false
    }
    return true
}

// Merge the common target configuration with the selected debug/release configuration.
// The Bool reports whether the merge succeeded; None means that targetName has no configuration.
func resolveTargetInfo(path: String, targetConfigMap: HashMap<String, TargetConfig>, targetName: String,
    isDebug: Bool, isTest: Bool): (Bool, ?TargetInfo) {
    if (!targetConfigMap.contains(targetName)) {
        return (true, None)
    }

    let targetConfig = targetConfigMap[targetName]
    let targetInfo = targetConfig.commonTargetInfo
    let profileTargetInfo = if (isDebug) {
        targetConfig.debugTargetInfo
    } else {
        targetConfig.releaseTargetInfo
    }
    if (!replaceTargetInfo(path, targetInfo, profileTargetInfo, targetName, isTest)) {
        return (false, None)
    }
    return (true, Some(targetInfo))
}

// Apply a resolved target configuration to the current module/workspace configuration.
// isTest controls whether profile-specific test dependencies enter the resolved target,
// while mergeTestDependencies controls whether those dependencies are copied into oriInfo.
// Keeping them separate preserves the different behavior required by TomlInfo and CommonInfo.
func mergeTargetConfig(path: String, oriInfo: TargetInfo, targetConfigMap: HashMap<String, TargetConfig>,
    targetName: String, isDebug: Bool, isTest: Bool, mergeTestDependencies: Bool): Bool {
    let (success, resolvedTargetInfo) = resolveTargetInfo(path, targetConfigMap, targetName, isDebug, isTest)
    if (!success) {
        return false
    }
    let targetInfo = resolvedTargetInfo ?? return true
    return replaceTargetInfo(path, oriInfo, targetInfo, targetName, mergeTestDependencies)
}

public func verifyTargetConfig(targetConfigMap: HashMap<String, TargetConfig>, path: String, configFilePath: String,
    isTest!: Bool = false): Bool {
    if (targetConfigMap.contains(targetConfigName) && (!verifyTargetInfo(
        targetConfigMap[targetConfigName].commonTargetInfo, path, configFilePath, isTest) ||
        !verifyTargetInfo(targetConfigMap[targetConfigName].debugTargetInfo, path, configFilePath, isTest) ||
        !verifyTargetInfo(targetConfigMap[targetConfigName].releaseTargetInfo, path, configFilePath, isTest))) {
        return false
    }
    if (!crossCompileTarget.isEmpty() && targetConfigMap.contains(crossCompileTarget) && (!verifyTargetInfo(
        targetConfigMap[crossCompileTarget].commonTargetInfo, path, configFilePath, isTest) ||
        !verifyTargetInfo(targetConfigMap[crossCompileTarget].debugTargetInfo, path, configFilePath, isTest) ||
        !verifyTargetInfo(targetConfigMap[crossCompileTarget].releaseTargetInfo, path, configFilePath, isTest))) {
        return false
    }
    return true
}

func verifyTargetInfo(targetInfo: TargetInfo, path: String, configFilePath: String, isTest: Bool): Bool {
    if (!safeCheck(targetInfo.compileOption, configFilePath) ||
        !safeCheck(targetInfo.overrideCompileOption, configFilePath) || !safeCheck(targetInfo.linkOption, configFilePath
    )) {
        return false
    }
    if (!verifyBinDependencies(targetInfo, path, configFilePath)) {
        return false
    }
    if (!verifyDependencyConfig(targetInfo.dependencies,
        "'target.${targetInfo.targetName}.dependencies' field at ${configFilePath}", path)) {
        return false
    }
    if (isTest &&
        !verifyDependencyConfig(targetInfo.testDependencies,
        "'target.${targetInfo.targetName}.test-dependencies' field at ${configFilePath}", path)) {
        return false
    }
    if (!verifyDependencyConfig(targetInfo.scriptDependencies,
        "'target.${targetInfo.targetName}.script-dependencies' field at ${configFilePath}", path)) {
        return false
    }
    if (!verifyDependencyConfig(targetInfo.replaceDeps,
        "'target.${targetInfo.targetName}.replace' field at ${configFilePath}", path)) {
        return false
    }
    if (!verifyCLibConfig(targetInfo.ffi.cLibs, path, configFilePath, targetString: "target.${targetInfo.targetName}.")) {
        return false
    }

    return true
}

func verifyBinDependencies(targetInfo: TargetInfo, path: String, configFilePath: String): Bool {
    var flag = true
    var binDependencies = targetInfo.binDependencies

    for (k in 0..binDependencies.pathOption.size) {
        // Path normalization in binDependencies.
        binDependencies.pathOption[k] = getPath(path, binDependencies.pathOption[k])
        if (!safeCheck(binDependencies.pathOption[k], configFilePath)) {
            return false
        }
        if (!directoryExists(binDependencies.pathOption[k])) {
            eprintln("Error: can not find path '${binDependencies.pathOption[k]}' which is listed in " +
                "'target.${targetInfo.targetName}.bin-dependencies' field at ${configFilePath}")
            flag = false
        }
    }

    for ((k, _) in binDependencies.packageOption) {
        if (k.isEmpty()) {
            eprintln("Error: the package name can't be empty which is listed in " +
                "'target.${targetInfo.targetName}.bin-dependencies' field at ${configFilePath}")
            flag = false
            continue
        }

        // Path normalization in binDependencies.
        binDependencies.packageOption[k] = getPath(path, binDependencies.packageOption[k])
        if (!safeCheck(k, configFilePath)) {
            return false
        }
        if (!safeCheck(binDependencies.packageOption[k], configFilePath)) {
            return false
        }

        // File exist Check.
        if (!fileExists(binDependencies.packageOption[k])) {
            eprintln("Error: can not find '${binDependencies.packageOption[k]}' which is listed in " +
                "'target.${targetInfo.targetName}.bin-dependencies' field at ${configFilePath}")
            flag = false
        }
    }
    return flag
}

/*
 * Normalize a target triple to its canonical form so that user-supplied aliases
 * can be matched against the native target reported by `cjc -v`.
 *
 * The rules below mirror cjc's own triple normalization:
 *
 *   - Architecture aliasing: `arm64` is an alias of `aarch64`.
 *
 *   - Vendor segment:
 *       * On Linux GNU / Windows / Android triples, `unknown` is optional and
 *         is treated as if it were always present (we normalize to the form
 *         that includes it).
 *       * On OHOS triples, `unknown` is also optional, but the canonical form
 *         is the one WITHOUT it (we normalize to the bare `<arch>-linux-ohos`).
 *       * On Windows triples, `pc` is optional in the same way.
 *
 *   - Windows / mingw triples: mingw-style names such as `x86_64-w64-mingw32`
 *     and `x86_64-pc-w64-mingw32` are equivalent to `x86_64-pc-windows-gnu`.
 *
 *   - Android API-level suffix (`-android<number>`, e.g. `android21`):
 *     the number is OPTIONAL syntactically (both `-android` and `-android21`
 *     are valid target names), but it is NOT a normalization wildcard:
 *       * A triple WITHOUT a number and a triple WITH a number are NOT equivalent.
 *       * Triples with DIFFERENT numbers are NOT equivalent.
 *     The full `-android[<number>]` suffix is therefore preserved verbatim in
 *     the normalized form.
 *
 *   - Apple iOS version suffix (`-ios<number-with-dot>`, e.g. `ios17.5`):
 *     same policy as Android. The version part is syntactically optional,
 *     but distinct versions (including the empty version) are NOT equivalent,
 *     so the `-ios[<number-with-dot>]` suffix is preserved verbatim. The same
 *     policy applies to the iOS simulator variant ending in `-simulator`.
 *
 * If the input does not match any known pattern, it is returned unchanged so
 * that callers can fall back to literal comparison.
 */
public func normalizeTarget(target: String): String {
    let parts = target.split("-")
    if (parts.size < 2) {
        return target
    }

    let arch = if (parts[0] == "arm64") {
        "aarch64"
    } else {
        parts[0]
    }

    var hasW64 = false
    var hasMingw32 = false
    var hasWindows = false
    var hasApple = false
    var hasLinux = false
    for (p in parts) {
        match (p) {
            case "w64" => hasW64 = true
            case "mingw32" => hasMingw32 = true
            case "windows" => hasWindows = true
            case "apple" => hasApple = true
            case "linux" => hasLinux = true
            case _ => ()
        }
    }

    if (hasWindows || (hasW64 && hasMingw32)) {
        return "${arch}-pc-windows-gnu"
    }

    let last = parts[parts.size - 1]

    if (hasLinux) {
        if (last == "gnu") {
            return "${arch}-unknown-linux-gnu"
        }
        if (last == "ohos") {
            return "${arch}-linux-ohos"
        }
        // `-android[<number>]`: keep the suffix verbatim, since distinct
        // numbers (and the empty number) are NOT equivalent triples.
        if (last.startsWith("android")) {
            return "${arch}-linux-${last}"
        }
    }

    if (hasApple) {
        if (last == "darwin") {
            return "${arch}-apple-darwin"
        }
        if (last == "simulator" && parts.size >= 3) {
            let osPart = parts[parts.size - 2]
            return "${arch}-apple-${osPart}-simulator"
        }
        // `-ios[<number-with-dot>]`: keep the suffix verbatim, since distinct
        // versions (and the empty version) are NOT equivalent triples.
        if (last.startsWith("ios")) {
            return "${arch}-apple-${last}"
        }
    }

    return target
}

public func combineDependencies(path: String, oriDeps: HashMap<String, DepInfo>, newDeps: HashMap<String, DepInfo>,
    targetName: String): Bool {
    for ((depName, newDep) in newDeps) {
        if (oriDeps.contains(depName)) {
            var oriDep = oriDeps[depName]
            if (!verifyDependencyField(oriDep, depName, path) || !verifyDependencyField(newDep, depName, path)) {
                return false
            }
            let (flag, errInfo) = oriDep.isAlternative(newDep)
            if (!errInfo.isEmpty()) {
                eprintln("Error: modules with name '${depName}' in dependency tree are conflicted. Please check " +
                    "'${Path(path).join(CONFIG_FILE_NAME).toString()}' when target=${targetName}")
                eprintln(errInfo)
                return false
            } else if (flag) {
                oriDeps.add(depName, newDep)
            }
        } else {
            oriDeps.add(depName, newDep)
        }
    }

    return true
}
