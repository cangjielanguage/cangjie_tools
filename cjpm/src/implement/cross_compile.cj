// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

// Append native data for cross-compile.
func appendNativeData(buildConfig: BuildConfig, data: ModuleResolve): Unit {
    if (!buildConfig.isCrossCompile) {
        return
    }

    for (item in data.resolves) {
        if (!buildConfig.packageList.macros.contains(item.fullName) &&
            buildConfig.pkgRequireByMacro.contains(item.fullName)) {
            item.nativePlatform = true
            continue
        }
        if (buildConfig.packageList.macros.contains(item.fullName)) {
            item.nativePlatform = true
            if (!buildConfig.targetMacroMods.contains(item.rootPkgName) &&
                !buildConfig.targetMacroPkgs.contains(item.fullName)) {
                item.targetPlatform = false
                continue
            }
        }
    }
    return
}

func collectMacroCjoTime(buildConfig: BuildConfig, pkgName: String): Unit {
    let rootPkgName: String = pkgName.split(".")[0]
    let nativeCjoFile: String = Path(buildConfig.globalConfig.nativeDir)
        .join(rootPkgName)
        .join("${pkgName}.cjo")
        .toString()
    if (!fileExists(nativeCjoFile)) {
        buildConfig.rebuildList.add(pkgName)
        return
    }

    if (buildConfig.targetMacroMods.contains(rootPkgName) || buildConfig.targetMacroPkgs.contains(pkgName)) {
        let targetCjoFile: String = Path(buildConfig.globalConfig.targetDir)
            .join(rootPkgName)
            .join("${pkgName}.cjo")
            .toString()
        if (!fileExists(targetCjoFile)) {
            buildConfig.rebuildList.add(pkgName)
        } else {
            buildConfig.cjoTimeMap[pkgName] = max(
                FileInfo(nativeCjoFile).lastModificationTime.toUnixTimeStamp().toSeconds(),
                FileInfo(targetCjoFile).lastModificationTime.toUnixTimeStamp().toSeconds())
        }
    } else {
        buildConfig.cjoTimeMap[pkgName] = FileInfo(nativeCjoFile).lastModificationTime.toUnixTimeStamp().toSeconds()
    }
}

func appendBinOptionForTest(res: ModuleResolve, requires: HashSet<String>, isLto: Bool): ArrayList<String> {
    let binOptions = ArrayList<String>()
    for ((_, eachModuleDepMap) in res.binDeps) {
        for (depName in eachModuleDepMap.packageRequiresSort) {
            if (!requires.contains(depName)) {
                continue
            }
            let info = eachModuleDepMap.packageRequires[depName]
            if (info.isMacroPackage) {
                continue
            }
            binOptions.add(all: linkPackage(info.libName, info.libPath, isLto), at: 0)
        }
    }
    for ((_, eachModuleDepMap) in res.crossBinDeps) {
        for (depName in eachModuleDepMap.packageRequiresSort) {
            if (!requires.contains(depName)) {
                continue
            }
            let info = eachModuleDepMap.packageRequires[depName]
            if (info.isMacroPackage) {
                continue
            }
            binOptions.add(all: linkPackage(info.libName, info.libPath, isLto), at: 0)
        }
    }
    return binOptions
}
