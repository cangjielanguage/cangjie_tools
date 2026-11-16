// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.Path
import cjpm.config.*

// LTO/combine validation and user-facing messages, grouped by build pipeline phase:
//   1. dep-resolve  — checkProjectCombineAtDepResolve
//   2. dep-model    — validateSuperPkgEntries
//   3. build-config — configureLtoAndCombineBuild, validateProjectCombineRootDependency
//
// State mutation lives in lto_combine_state.cj; platform checks in lto_combine_capabilities.cj.

private func combineAllDepsField(rootName: String): String {
    "'package.package-configuration.${rootName}.combine-all-deps = true'"
}

// Prefer struct lto.level over deprecated string profile.build.lto.
func resolveLtoLevelFromToml(tomlInfo: TomlInfo): String {
    if (!tomlInfo.profile.build.lto.level.isEmpty()) {
        tomlInfo.profile.build.lto.level
    } else {
        tomlInfo.profile.build.ltoValue
    }
}

func warnLtoDisablesCompilePipelineParallel(): Unit {
    println("Warning: cjpm won't apply pipeline optimization to the build process when field 'profile.build.lto' is 'full' or 'thin'")
}

// Dep-resolve phase: validate and set isProjectCombine on globalConfig.
func checkProjectCombineAtDepResolve(globalConfig: GlobalConfig, tomlInfo: TomlInfo): Bool {
    if (!tomlInfo.pkg.packageConfiguration.contains(globalConfig.rootName) ||
        !tomlInfo.pkg.packageConfiguration[globalConfig.rootName].combineAllDeps) {
        return true
    }
    if (tomlInfo.profile.build.combined.isEmpty()) {
        eprintln("Error: field ${combineAllDepsField(globalConfig.rootName)} " +
            "is invalid when field 'profile.build.combined' is empty")
        return false
    }
    if (globalConfig.isForScript) {
        println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
            "combine-all-deps module when depended by build.cj")
    } else if (globalConfig.isTest) {
        println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
            "combine-all-deps module for test")
    } else {
        globalConfig.isProjectCombine = true
    }
    true
}

// Build-config phase: apply LTO settings and validate lto-combine prerequisites.
func configureLtoAndCombineBuild(buildConfig: BuildConfig, tomlInfo: TomlInfo): Bool {
    let rootName = buildConfig.globalConfig.rootName
    if (buildConfig.globalConfig.isProjectCombine &&
        buildConfig.packageList.isMacro(rootName)) {
        eprintln("Error: field ${combineAllDepsField(rootName)} " +
            "is not support for macro root package '${rootName}'")
        return false
    }

    if (!isLtoConfiguredInToml(tomlInfo) && tomlInfo.profile.build.lto.keepPkgVisibility.isSome()) {
        eprintln("Error: field 'keep-pkg-visibility' of '[profile.build.lto]' is not supported when lto is disabled")
        return false
    }

    if (isLtoConfiguredInToml(tomlInfo)) {
        buildConfig.isLto = supportLtoForCurrentTarget()
        if (!buildConfig.isLto) {
            println("Warning: configuration failed, field 'profile.build.lto' " +
                "settings only support compilation for ${LTO_SUPPORTED_PLATFORMS}")
        }
        if (!tomlInfo.profile.build.ltoValue.isEmpty()) {
            println("Warning: The string-type 'lto' under 'profile.build' will be deprecated. " +
                "Please use the struct-type 'lto' to enable LTO functionality. For details, refer to the user guide.")
        }
        buildConfig.ltoValue = resolveLtoLevelFromToml(tomlInfo)
        buildConfig.keepPkgVisibility = tomlInfo.profile.build.lto.keepPkgVisibility
        if (let Some(keepPkgs) <- buildConfig.keepPkgVisibility) {
            for (fullName in keepPkgs) {
                if (!keepPkgVisibilityNameCheck(fullName)) {
                    return false
                }
            }
        }
    }

    if (supportLtoForCurrentTarget() && buildConfig.globalConfig.isProjectCombine &&
        buildConfig.ltoValue.isEmpty()) {
        eprintln("Error: field ${combineAllDepsField(rootName)} " +
            "is invalid when field 'profile.build.lto' is empty")
        return false
    } else if (!supportLtoForCurrentTarget() && buildConfig.globalConfig.isProjectCombine) {
        eprintln("Error: field ${combineAllDepsField(rootName)} " +
            "only supports compilation to ${LTO_SUPPORTED_PLATFORMS}")
        return false
    }

    if (isIosTarget(getCompileTarget()) && !buildConfig.globalConfig.isProjectCombine &&
        !tomlInfo.profile.build.combined.isEmpty() && !buildConfig.isLto) {
        reportIosCombinedRequiresLto()
        return false
    }
    true
}

// Build-config phase: root package must not be depended on when combine-all-deps is enabled.
func validateProjectCombineRootDependency(buildConfig: BuildConfig, depPkgName: String,
    depRequires: HashSet<String>): Bool {
    if (!buildConfig.globalConfig.isProjectCombine) {
        return true
    }
    let rootName = buildConfig.globalConfig.rootName
    if (depRequires.contains(rootName)) {
        eprintln("Error: root package '${rootName}' is depended by package ${depPkgName}, " +
            "it cannot be depended by any package when field 'combine-all-deps = true' is valid")
        return false
    }
    true
}

// Missing combined configuration is an error; falling back to package.output-type would hide it.
func getConfiguredCombinedOutputType(tomlInfo: TomlInfo, moduleName: String): ?OutputType {
    let outputType = tomlInfo.profile.build.combined.get(moduleName)
    if (outputType.isNone()) {
        eprintln("Error: field 'profile.build.combined.${moduleName}' must be configured in " +
            "${Path(tomlInfo.pkg.path).join(CONFIG_FILE_NAME)}")
    }
    outputType
}

// Only iOS project roots choose their output type; other platforms keep dynamic output.
// For builds without project combine, the returned default is unused.
func resolveProjectCombineOutputType(buildConfig: BuildConfig, tomlInfo: TomlInfo): ?OutputType {
    if (!buildConfig.globalConfig.isProjectCombine || !isIosTarget(getCompileTarget())) {
        return Dynamic
    }
    getConfiguredCombinedOutputType(tomlInfo, buildConfig.globalConfig.rootName)
}

// All selected workspace entries must agree before propagating their combined output type.
func resolveIosCombinedOutputType(dm: DepModel): ?OutputType {
    if (!COMMON_INFO.inWorkspace) {
        let rootName = dm.globalConfig.rootName
        if (let Some(rootModule) <- dm.depAnalysisEngine.finalDepMap.get(rootName)) {
            return getConfiguredCombinedOutputType(rootModule.tomlInfo, rootName)
        }
        eprintln("Error: cannot find source module '${rootName}' for iOS lto-combine")
        return None
    }
    // Match selected members and resolved modules using canonical paths.
    let memberPaths = HashSet<String>()
    for (memberPath in COMMON_INFO.currentMembers) {
        memberPaths.add(getCanonicalPath(memberPath))
    }
    var outputType: ?OutputType = None
    for ((moduleName, finalDep) in dm.depAnalysisEngine.finalDepMap) {
        let memberPath = getCanonicalPath(finalDep.tomlInfo.pkg.path)
        if (!memberPaths.contains(memberPath)) {
            continue
        }
        let memberType = getConfiguredCombinedOutputType(finalDep.tomlInfo, moduleName) ?? return None
        if (let Some(previousType) <- outputType && previousType != memberType) {
            eprintln("Error: selected workspace members must configure the same 'profile.build.combined' output type on iOS")
            return None
        }
        outputType = memberType
        memberPaths.remove(memberPath)
    }
    // Remaining paths identify selected members missing from the dependency graph.
    if (!memberPaths.isEmpty() || outputType.isNone()) {
        eprintln("Error: cannot resolve all selected workspace members for iOS lto-combine")
        return None
    }
    outputType
}

func warnIosCombinedOutputType(pkgName: String, outputType: OutputType, combinedType: OutputType): Unit {
    if (outputType != combinedType) {
        println("Warning: output-type of combined root package '${pkgName}' is overridden by " +
            "'profile.build.combined'; final output will be '${combinedType.toString()}'")
    }
}

func validateCombinedRootHasSource(superPkgName: String): Bool {
    eprintln("Error: field 'profile.build.combined' is not supported for root package '${superPkgName}' " +
        "which contain no source code for compilation")
    false
}

func warnCombinedMacroRoot(superPkgName: String): Unit {
    println("Warning: field 'profile.build.combined' is not supported for macro root package '${superPkgName}'")
}

func warnCombinedPlatformUnsupported(superPkgName: String): Unit {
    println("Warning: field 'profile.build.combined' only supports '${MODULE_COMBINE_SUPPORTED_PLATFORMS}', " +
        "so output targeted for module '${superPkgName}' will not be combined")
}

func warnCombinedRootOutputTypeNotDynamic(superPkgName: String, configFilePath: String): Unit {
    println("Warning: output-type of combined root package '${superPkgName}' in " +
        "${configFilePath} is not 'dynamic', " +
        "final output will be dynamic library")
}

func isValidCombinedEntry(dm: DepModel, pkgName: String, hasLto: Bool): Bool {
    if (!dm.packageDepMap.contains(pkgName)) {
        return false
    }
    if (dm.packageDepMap[pkgName].isMacroPackage) {
        return false
    }
    if (!supportModuleCombineForCurrentTarget(hasLto)) {
        return false
    }
    true
}

func reportInvalidCombinedEntry(dm: DepModel, pkgName: String, hasLto: Bool): Unit {
    if (!dm.packageDepMap.contains(pkgName)) {
        eprintln("Error: field 'profile.build.combined' is not supported for root package '${pkgName}' " +
            "which contain no source code for compilation")
        return
    }
    if (dm.packageDepMap[pkgName].isMacroPackage) {
        eprintln("Error: field 'profile.build.combined' is not supported for macro root package '${pkgName}'")
        return
    }
    if (isIosTarget(getCompileTarget()) && !hasLto) {
        reportIosCombinedRequiresLto()
        return
    }
    if (!supportModuleCombineForCurrentTarget(hasLto)) {
        eprintln("Error: field 'profile.build.combined' only supports '${MODULE_COMBINE_SUPPORTED_PLATFORMS}', " +
            "so output targeted for module '${pkgName}' cannot be combined")
        return
    }
}

func isCombinedEntryConfigured(dm: DepModel): Bool {
    if (!COMMON_INFO.inWorkspace) {
        return dm.rootConfiguredCombined
    }
    if (COMMON_INFO.currentMembers.isEmpty()) {
        return false
    }
    for (memberPath in COMMON_INFO.currentMembers) {
        if (!dm.combinedConfiguredMembers.contains(getCanonicalPath(memberPath))) {
            return false
        }
    }
    true
}

func validateCombinedEntry(dm: DepModel, hasLto: Bool): Bool {
    var entryValid = true
    if (!COMMON_INFO.inWorkspace) {
        if (!isValidCombinedEntry(dm, dm.globalConfig.rootName, hasLto)) {
            reportInvalidCombinedEntry(dm, dm.globalConfig.rootName, hasLto)
            entryValid = false
        }
    } else {
        for (memberPath in COMMON_INFO.currentMembers) {
            let canonicalMemberPath = getCanonicalPath(memberPath)
            for ((moduleName, finalDep) in dm.depAnalysisEngine.finalDepMap) {
                if (getCanonicalPath(finalDep.tomlInfo.pkg.path) == canonicalMemberPath &&
                    !isValidCombinedEntry(dm, moduleName, hasLto)) {
                    reportInvalidCombinedEntry(dm, moduleName, hasLto)
                    entryValid = false
                }
            }
        }
    }
    entryValid
}

func shouldSkipModuleCombineOnPlatform(compileTarget: String, isProjectCombine: Bool, hasLto: Bool): Bool {
    !supportModuleCombine(compileTarget, isProjectCombine, hasLto)
}

// Returns modules that should be removed from superPkgMap (macro root or unsupported platform).
func validateSuperPkgEntries(dm: DepModel, isProjectCombine: Bool, compileTarget: String,
    configFilePath: String, hasLto: Bool): (Bool, HashSet<String>) {
    let notSupportSet = HashSet<String>()
    var superPkgInValidFlag = false
    if (!dm.superPkgMap.isEmpty() && isIosTarget(compileTarget) && !isProjectCombine && !hasLto) {
        reportIosCombinedRequiresLto()
        return (false, notSupportSet)
    }
    for ((superPkgName, _) in dm.superPkgMap) {
        if (!dm.packageDepMap.contains(superPkgName)) {
            superPkgInValidFlag = true
            validateCombinedRootHasSource(superPkgName)
            continue
        }
        if (dm.packageDepMap[superPkgName].isMacroPackage) {
            warnCombinedMacroRoot(superPkgName)
            notSupportSet.add(superPkgName)
            continue
        }
        if (shouldSkipModuleCombineOnPlatform(compileTarget, isProjectCombine, hasLto)) {
            warnCombinedPlatformUnsupported(superPkgName)
            notSupportSet.add(superPkgName)
            continue
        }
    }
    (!superPkgInValidFlag, notSupportSet)
}
