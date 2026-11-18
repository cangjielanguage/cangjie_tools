// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

// LTO/combine state mutations on DepModel and ResolveItem.
// Validation is done in lto_combine_policy.cj before calling into this module.

// For one combined module root: collect sub-packages, force them to Static.
// The entry combined configuration determines the root output type.
func applyModuleCombineForSuperPkg(dm: DepModel, superPkgName: String, superPkgCfg: SuperPackageConfig,
    compileTarget: String, configFilePath: String): Unit {
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
    if (isIosTarget(compileTarget)) {
        warnIosCombinedOutputType(superPkgName, dm.packageDepMap[superPkgName].outputType, superPkgCfg.outputType)
        dm.packageDepMap[superPkgName].outputType = superPkgCfg.outputType
    } else if (dm.packageDepMap[superPkgName].outputType != Dynamic) {
        warnCombinedRootOutputTypeNotDynamic(superPkgName, configFilePath)
        dm.packageDepMap[superPkgName].outputType = Dynamic
    }
}

// Dep-resolve phase: validate then apply module-level combine state.
// hasLto comes from the build root, which also controls compilation for workspace members.
func applySuperPkgCombineState(dm: DepModel, isProjectCombine: Bool, hasLto: Bool): Bool {
    let compileTarget = getCompileTarget()
    let configFilePath = Path(dm.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString()

    if (isProjectCombine) {
        let (valid, _) = validateSuperPkgEntries(dm, isProjectCombine, compileTarget, configFilePath, hasLto)
        if (valid && isIosTarget(compileTarget)) {
            let rootName = dm.globalConfig.rootName
            if (!dm.packageDepMap.contains(rootName)) {
                return validateCombinedRootHasSource(rootName)
            }
            let combinedType = resolveIosCombinedOutputType(dm) ?? return false
            warnIosCombinedOutputType(rootName, dm.packageDepMap[rootName].outputType, combinedType)
        }
        return valid
    }

    // Keep main-branch silent-disable semantics: profile.build.combined is controlled by
    // the entry module (or all selected workspace members). A dependency's own setting
    // must not enable combine when the entry did not configure it.
    if (!isCombinedEntryConfigured(dm)) {
        return true
    }

    if (isIosTarget(compileTarget) && !hasLto) {
        reportIosCombinedRequiresLto()
        return false
    }

    // Entry configured combined: propagate to all source modules in the dependency graph.
    if (!validateCombinedEntry(dm, hasLto)) {
        return false
    }
    // Resolve once from the entry; dependency modules must not choose a different root type.
    let rootOutputType = if (isIosTarget(compileTarget)) {
        resolveIosCombinedOutputType(dm) ?? return false
    } else { Dynamic }
    for ((moduleName, _) in dm.depAnalysisEngine.finalDepMap) {
        if (!dm.packageDepMap.contains(moduleName)) {
            continue
        }
        let superPkgCfg = SuperPackageConfig(moduleName, rootOutputType)
        applyModuleCombineForSuperPkg(dm, moduleName, superPkgCfg, compileTarget, configFilePath)
        dm.superPkgMap[moduleName] = superPkgCfg
    }
    dm.checkSuperPkgDep()
    true
}

// Ensure root package is built last when project combine is enabled.
func applyProjectCombineBuildOrder(dm: DepModel, rootName: String): Unit {
    dm.readyPkgs.removeIf {v => v == rootName}
    dm.readyPkgs.add(rootName)
}

// Combined root packages link sub-packages via forcedRequires; strip the duplicate entries.
func stripSuperPkgForcedRequires(dm: DepModel): Unit {
    for ((superPkgName, _) in dm.superPkgMap) {
        dm.packageDepMap[superPkgName].requires.remove(all: dm.packageDepMap[superPkgName].forcedRequires)
    }
}

// Build-config phase: project lto-combine assigns Static to deps, the configured type to the iOS root.
func applyProjectCombinePackageOutputType(buildConfig: BuildConfig, pkgName: String, pkg: ResolveItem,
    rootOutputType: OutputType): Unit {
    if (!buildConfig.globalConfig.isProjectCombine) {
        return
    }
    let rootName = buildConfig.globalConfig.rootName
    let compileTarget = getCompileTarget()
    if (pkgName != rootName && !pkg.isMacroPackage) {
        pkg.outputType = Static
    } else if (isIosTarget(compileTarget) && pkgName == rootName) {
        pkg.outputType = rootOutputType
    } else {
        pkg.outputType = Dynamic
    }
}
