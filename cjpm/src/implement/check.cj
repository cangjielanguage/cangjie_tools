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

    let (checkSuccessful, mockUsed) = dm.resolve(globalConfig.rootPath, lockFile.requires)
    if (!checkSuccessful) {
        return (dm, false, mockUsed)
    }

    // Don't lock path dependencies since it would only cause problems for transitive path dependencies
    dm.depMap.removeIf {_, v => v.path.isSome()}

    if (!isScriptDep) {
        var lockFilePath: String = Path(globalConfig.rootPath).join(LOCK_FILE_NAME).toString()
        // Overwrite file only if we have meaningful changes in case field serialization order got changed
        if (lockFile.requires != dm.depMap || !fileExists(lockFilePath)) {
            if (!lockFile.requires.isEmpty() && !deleteFile(lockFilePath)) {
                return (dm, false, mockUsed)
            }

            if (!creatLockFile(globalConfig.rootPath, ModuleLock(dm.depMap, lockFile.scripts))) {
                return (dm, false, mockUsed)
            }
        }
    }

    dealWithSuperPkg(dm)

    // Circular dependency check.
    if (!topoSort(dm)) {
        return (dm, false, mockUsed)
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

func dealWithSuperPkg(dm: DepModel): Unit {
    let notSupportSet = HashSet<String>()
    for ((superPkgName, superPkgCfg) in dm.superPkgMap) {
        if (dm.packageDepMap[superPkgName].isMacroPackage) {
            println("Warning: field 'profile.build.combined' is not supported for macro root package ${superPkgName}")
            notSupportSet.add(superPkgName)
            continue
        }
        if (isCombinedForMac()) {
            println("Warning: field 'profile.build.combined' is not supported on macOS, " +
                "so output targeted for macOS of module '${superPkgName}' will not be combined")
            notSupportSet.add(superPkgName)
            continue
        }
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
    }
    for (notSupport in notSupportSet) {
        dm.superPkgMap.remove(notSupport)
    }

    dm.checkSuperPkgDep()
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

@When[os == "macOS"]
func isCombinedForMac(): Bool {
    return crossCompileTarget.isEmpty() || crossCompileTarget.contains("darwin")
}

@When[os != "macOS"]
func isCombinedForMac(): Bool {
    return crossCompileTarget.contains("darwin")
}