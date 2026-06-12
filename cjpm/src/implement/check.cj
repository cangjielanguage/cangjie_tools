// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

// Dependency resolve.
func resolve(globalConfig: GlobalConfig, isScriptDep!: Bool = false): (DepModel, Bool, Bool) {
    var dm = DepModel(globalConfig)

    let (loadFlag, lockFile) = if (isScriptDep) {
        (true, ModuleLock())
    } else {
        loadLockFile(globalConfig.rootPath)
    }
    if (!loadFlag) {
        return (dm, false, false)
    }

    let (checkSuccessful, mockUsed) = dm.resolve(globalConfig.rootPath, lockFile.requires, isScriptDep)
    if (!checkSuccessful) {
        return (dm, false, mockUsed)
    }

    let depMap = dm.depAnalysisEngine.getDepMap()
    // Don't lock path dependencies since it would only cause problems for transitive path dependencies
    depMap.removeIf {_, v => v.path.isSome()}

    if (!isScriptDep) {
        var lockFilePath: String = Path(globalConfig.rootPath).join(LOCK_FILE_NAME).toString()
        // Overwrite file only if we have meaningful changes in case field serialization order got changed
        if (lockFile.requires != depMap || !fileExists(lockFilePath)) {
            if (!lockFile.requires.isEmpty() && !deleteFile(lockFilePath)) {
                return (dm, false, mockUsed)
            }

            if (!creatLockFile(globalConfig.rootPath, ModuleLock(depMap, lockFile.scripts))) {
                return (dm, false, mockUsed)
            }
        }
    }

    // combined is only used in cjpm build
    if (!dm.globalConfig.isPureBuild) {
        dm.superPkgMap.clear()
    } else if (!dealWithSuperPkg(dm, globalConfig.isProjectCombine)) {
        return (dm, false, mockUsed)
    }

    // Circular dependency check.
    if (!topoSort(dm)) {
        return (dm, false, mockUsed)
    }

    if (globalConfig.isProjectCombine) {
        dm.readyPkgs.removeIf {v => v == globalConfig.rootName}
        dm.readyPkgs.add(globalConfig.rootName)
    }

    // Check parent-child package accessLevel.
    if (!checkAccessLevel(dm)) {
        return (dm, false, mockUsed)
    }

    for ((superPkgName, _) in dm.superPkgMap) {
        dm.packageDepMap[superPkgName].requires.remove(all: dm.packageDepMap[superPkgName].forcedRequires)
    }

    return (dm, true, mockUsed)
}

func isValidSuperPkgEntry(dm: DepModel, pkgName: String): Bool {
    if (!dm.packageDepMap.contains(pkgName)) {
        return false
    }
    if (dm.packageDepMap[pkgName].isMacroPackage) {
        return false
    }
    if (!isSupportCombine()) {
        return false
    }
    return true
}

func processSingleSuperPkg(dm: DepModel, superPkgName: String): Unit {
    if (!dm.packageDepMap.contains(superPkgName)) {
        eprintln("Error: field 'profile.build.combined' is not supported for root package '${superPkgName}' " +
            "which contain no source code for compilation")
        return
    }
    if (dm.packageDepMap[superPkgName].isMacroPackage) {
        println("Warning: field 'profile.build.combined' is not supported for macro root package '${superPkgName}'")
        return
    }
    if (!isSupportCombine()) {
        println("Warning: field 'profile.build.combined' only supports 'Linux/OHOS/Windows', " +
            "so output targeted for module '${superPkgName}' will not be combined")
        return
    }

    let superPkgCfg = SuperPackageConfig(superPkgName, Dynamic)
    for (pkgName in dm.packageDepMap.keys()) {
        if (superPkgName != pkgName.split(".")[0] || superPkgName == pkgName) {
            continue
        }
        superPkgCfg.subPkgSet.add(pkgName)
        if (!dm.packageDepMap[pkgName].isMacroPackage) {
            dm.packageDepMap[pkgName].outputType = Static
        }
    }
    dm.packageDepMap[superPkgName].superPkgCfg = superPkgCfg
    if (dm.packageDepMap[superPkgName].outputType != Dynamic) {
        println("Warning: output-type of combined root package '${superPkgName}' in " +
            "${Path(dm.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString()} is not 'dynamic', " +
            "final output will be dynamic library")
        dm.packageDepMap[superPkgName].outputType = Dynamic
    }
    dm.superPkgMap[superPkgName] = superPkgCfg
}

// Report why an entry (root/member) that configured combined is invalid.
// Any invalid entry fails the build — no half-effective propagation.
func reportInvalidSuperPkgEntry(dm: DepModel, pkgName: String): Unit {
    if (!dm.packageDepMap.contains(pkgName)) {
        eprintln("Error: field 'profile.build.combined' is not supported for root package '${pkgName}' " +
            "which contain no source code for compilation")
        return
    }
    if (dm.packageDepMap[pkgName].isMacroPackage) {
        eprintln("Error: field 'profile.build.combined' is not supported for macro root package '${pkgName}'")
        return
    }
    if (!isSupportCombine()) {
        eprintln("Error: field 'profile.build.combined' only supports 'Linux/OHOS/Windows', " +
            "so output targeted for module '${pkgName}' cannot be combined")
        return
    }
}

func dealWithSuperPkg(dm: DepModel, isProjectCombine: Bool): Bool {
    // Project-level combined has its own path; do not interfere.
    if (isProjectCombine) {
        return true
    }

    // 1. Did the entry explicitly configure combined? (fact only, no validity check here)
    var rootConfigured = false
    if (!COMMON_INFO.inWorkspace) {
        rootConfigured = dm.rootConfiguredCombined
    } else if (!COMMON_INFO.currentMembers.isEmpty()) {
        rootConfigured = true
        for (memberPath in COMMON_INFO.currentMembers) {
            if (!dm.combinedConfiguredMembers.contains(getCanonicalPath(memberPath))) {
                rootConfigured = false
                break
            }
        }
    }

    // 2. Entry did not configure combined: silently ignore all dependency-level configs, no error.
    //    (This is the core semantic e1d594e4 intended — keep it.)
    if (!rootConfigured) {
        return true
    }

    // 3. Entry configured combined but is itself invalid: fail the build.
    //    scheme B — no half-effective state where entry is skipped yet deps propagate.
    var entryValid = true
    if (!COMMON_INFO.inWorkspace) {
        if (!isValidSuperPkgEntry(dm, dm.globalConfig.rootName)) {
            reportInvalidSuperPkgEntry(dm, dm.globalConfig.rootName)
            entryValid = false
        }
    } else {
        for (memberPath in COMMON_INFO.currentMembers) {
            for ((moduleName, finalDep) in dm.depAnalysisEngine.finalDepMap) {
                if (getCanonicalPath(finalDep.tomlInfo.pkg.path) == getCanonicalPath(memberPath) &&
                    !isValidSuperPkgEntry(dm, moduleName)) {
                    reportInvalidSuperPkgEntry(dm, moduleName)
                    entryValid = false
                }
            }
        }
    }
    if (!entryValid) {
        return false
    }

    // 4. Entry configured and valid: propagate to all dependent source modules.
    //    processSingleSuperPkg does per-module validity checks (deps that are macro / unsupported
    //    get a Warning and are skipped — that's dependency-level, not entry-level).
    for ((moduleName, _) in dm.depAnalysisEngine.finalDepMap) {
        processSingleSuperPkg(dm, moduleName)
    }

    dm.checkSuperPkgDep()
    return true
}

public func doCheck(memberModule: String, testCheck: Bool): Bool {
    let tomlInfo: TomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    if (!checkForWorkspace(DIR_CURRENT, tomlInfo, memberModule, isTest: testCheck)) {
        return false
    }

    var globalConfig: GlobalConfig = GlobalConfig()
    globalConfig.isTest = testCheck
    let (dataModel, flag, _) = resolve(globalConfig)
    if (!flag) {
        return false
    }
    let res: String = String.join(dataModel.readyPkgs.toArray(), delimiter: " -> ")
    println("The valid serial compilation order is:\n    ${res}")
    return true
}

func checkAccessLevel(topoData: DepModel): Bool {
    var levelMap = HashMap<String, String>()
    for ((k, v) in topoData.packageDepMap) {
        levelMap[k] = v.accessLevel
    }

    var checkFlag: Bool = true
    for (subPkg in topoData.readyPkgs) {
        if (!subPkg.contains(".")) {
            continue
        }
        let parentPkg: String = getParentPkg(subPkg)
        if (!levelMap.contains(parentPkg) || !levelMap.contains(subPkg)) {
            continue
        }
        if (!checkParentChildPkg(parentPkg, levelMap[parentPkg], subPkg, levelMap[subPkg])) {
            checkFlag = false
        }
    }
    return checkFlag
}

func getParentPkg(subPkg: String): String {
    let pos = subPkg.lastIndexOf(".").getOrThrow()
    return subPkg[0..pos]
}

func checkParentChildPkg(parentPkg: String, parentLevel: String, subPkg: String, subLevel: String): Bool {
    let leverErr: Bool = (parentLevel == "protected" && subLevel == "public") ||
        (parentLevel == "internal" && subLevel != "internal")

    if (leverErr) {
        eprintln("Error: the access level of child package can't be higher than that of parent package")
        eprintln("    parent package '${parentPkg}' access level: '${parentLevel}'")
        eprintln("    child package '${subPkg}' access level: '${subLevel}'")
        return false
    }

    return true
}