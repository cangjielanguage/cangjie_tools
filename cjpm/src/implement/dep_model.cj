// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.sync.*
import std.fs.*
import std.sort.*
import stdx.encoding.json.*
import stdx.serialization.serialization.*
import cjpm.config.*
import cjpm.util.*

// left - whether the build check is successful
// right - whether mocks are used
type BuildCheckResult = (Bool, Bool)

let FAILED_BUILD_CHECK_RESULT = (false, false)

class DepModel {
    // Dep model config
    public var globalConfig: GlobalConfig = GlobalConfig()

    // Engine for dep-analysis
    public let depAnalysisEngine = DepAnalysisEngine()

    // All module dependency information for the current project.
    public let resolvedModuleMap: ResolveMap = ResolveMap()
    public let verifiedModuleMap: VerifiedMap = VerifiedMap()
    public let propagatedFeatures: HashMap<String, HashSet<Feature>> = HashMap<String, HashSet<Feature>>()
    // Replace map
    public var replaceMap: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    // Locked dep map
    public var lockedMap: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    // All package dependency information for the current project.
    public var packageDepMap: HashMap<String, ResolveItem> = HashMap<String, ResolveItem>()
    // All binary dependency information for the all module.
    public var binDepMap: HashMap<String, BinDeps> = HashMap<String, BinDeps>()
    public var binaryPkgList: HashMap<String, RequiresPackagesInfo> = HashMap<String, RequiresPackagesInfo>()
    // All binary dependency information for target platform of cross compile.
    public var crossBinDepMap: HashMap<String, BinDeps> = HashMap<String, BinDeps>()
    public var indegrees: HashMap<String, Int64> = HashMap<String, Int64>()
    public var outdegrees: HashMap<String, Int64> = HashMap<String, Int64>()
    public var edges: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var reverseEdges: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var visit: HashSet<String> = HashSet<String>()
    // Record package name for all modules
    public var sourceCodePkgList: ArrayList<String> = ArrayList<String>()
    // Record all module.json information.
    public var jsonMap: HashMap<String, ArrayList<String>> = HashMap<String, ArrayList<String>>()
    public var pkgCacheMap: HashMap<String, CacheItem> = HashMap<String, CacheItem>()
    let packageMtx: Mutex = Mutex()
    let errPrintMtx: Mutex = Mutex()
    let scanCjoMtx: Mutex = Mutex()
    let scanPkgMtx: Mutex = Mutex()
    // Collect zero indegree packages
    var readyPkgs: ArrayList<String> = ArrayList<String>()
    public var targetMacroPkgs: HashSet<String> = HashSet<String>() // used by cross compile
    public var targetMacroMods: HashSet<String> = HashSet<String>() // used by cross compile

    // Super package record
    public var superPkgMap: HashMap<String, SuperPackageConfig> = HashMap<String, SuperPackageConfig>()
    public var isCrossBin = false
    // Sub package map to super package
    public var superPkgSubMap: HashMap<String, String> = HashMap<String, String>()
    // Binary dependencies record
    public var binDepCacheMap: HashMap<String, RequiresPackages> = HashMap<String, RequiresPackages>()
    // Package dependencies record (pkg without test)
    public var pkgBuildDepCacheMap: HashMap<String, RequiresPackages> = HashMap<String, RequiresPackages>()
    // Package dependencies record (pkg with test)
    public var pkgTestDepCacheMap: HashMap<String, RequiresPackages> = HashMap<String, RequiresPackages>()

    public init(globalConfig: GlobalConfig) {
        this.globalConfig = globalConfig
    }

    public func getLen(): Int64 {
        return this.packageDepMap.size
    }

    public func forEach(fn: (String, HashMap<String, Int64>,
        HashMap<String, Int64>,HashMap<String, ArrayList<String>>) -> Unit): Unit {
        for ((name, _) in this.packageDepMap) {
            fn(name, this.indegrees, this.outdegrees, this.edges)
        }
        return
    }

    public func forEachDependency(name: String, fn: (String) -> Unit): Unit {
        if (this.packageDepMap.contains(name)) {
            this.packageDepMap[name].forEachDependency(fn)
        }
        return
    }

    public func resolve(path: String, locked: HashMap<String, DepInfo>, isScriptDep: Bool): BuildCheckResult {
        // initialize dep-analysis engine
        this.depAnalysisEngine.isTest = this.globalConfig.isTest
        this.depAnalysisEngine.isDebug = this.globalConfig.isDebug
        this.depAnalysisEngine.rootModulePath = this.globalConfig.modulePath

        // move repo-deps from cjpm.lock into engine
        this.depAnalysisEngine.recordLock(locked)

        // first process locked dependencies as if they are directly required.
        this.globalConfig.scriptList = ArrayList<ScriptConfig>()

        if (!COMMON_INFO.inWorkspace) {
            this.globalConfig.modulePath = this.globalConfig.rootPath
            this.lockedMap = locked
            if (this.depCheck(path, "", "", None) == FAILED_BUILD_CHECK_RESULT) {
                return FAILED_BUILD_CHECK_RESULT
            }
        } else {
            this.replaceMap = COMMON_INFO.replaceDeps
            let (flag, _) = this.processDependencyModule(locked, "", "", true)
            if (!flag) {
                return FAILED_BUILD_CHECK_RESULT
            }
        }

        if (!COMMON_INFO.memberModule.isEmpty()) {
            COMMON_INFO.currentMembers = ArrayList<String>([COMMON_INFO.memberModule])
        } else if (this.globalConfig.resolveType == RESOLVE_TEST) {
            COMMON_INFO.currentMembers = COMMON_INFO.testMembers
        } else if (this.globalConfig.resolveType == RESOLVE_BUILD) {
            COMMON_INFO.currentMembers = COMMON_INFO.buildMembers
        } else {
            COMMON_INFO.currentMembers = COMMON_INFO.members
        }

        var mockUsed: Bool = false
        var checkFlag: Bool = true
        for (member in COMMON_INFO.currentMembers) {
            // repo-dep from script-dependencies
            if (isScriptDep && member.contains(SCRIPT_DEPENDENCIES_MARK)) {
                let repoDepArr = member.split(SCRIPT_DEPENDENCIES_MARK)
                let nameVersionArr = repoDepArr[1].split("-")
                let depInfo = DepInfo()
                depInfo.importedModuleName = BUILD_SCRIPT_NAME
                depInfo.importedPath = repoDepArr[0]
                depInfo.version = nameVersionArr[1]
                this.depAnalysisEngine.repoDepList.add((nameVersionArr[0], depInfo))
                continue
            }

            this.globalConfig.modulePath = member
            let (checkStatus, mockFlag) = this.depCheck(member, "", "", None)
            checkFlag = checkFlag && checkStatus
            mockUsed = mockUsed || mockFlag
        }
        if (!checkFlag) {
            return (checkFlag, mockUsed)
        }

        // get all dependencies from central repository
        this.depAnalysisEngine.replaceMap = this.replaceMap
        if (!this.depAnalysisEngine.analyseRepoDeps()) {
            return FAILED_BUILD_CHECK_RESULT
        }

        // deal with replaced module
        let replacedRequire = HashMap<String, DepInfo>()
        for (name in this.depAnalysisEngine.replacedModuleSet) {
            replacedRequire.add(name, this.replaceMap[name])
        }
        let (flag, _) = this.processDependencyModule(replacedRequire, this.globalConfig.rootPath, this.globalConfig.rootName, true)
        if (!flag) {
            return FAILED_BUILD_CHECK_RESULT
        }

        // deal with all dependencies from central repository
        for ((_, finalDep) in this.depAnalysisEngine.finalDepMap) {
            if (finalDep.sourceInfo.isCenterRepoDep() && !this.repoDepCheck(finalDep)) {
                return FAILED_BUILD_CHECK_RESULT
            }
        }

        this.globalConfig.verifiedEnabledFeatures = verifyCliEnabledFeatures(this.globalConfig.enabledFeatures, this.resolvedModuleMap)
            .inspectErr { errs => for (err in errs) { eprintln("Error: ${err}") } }
            .ok ?? return FAILED_BUILD_CHECK_RESULT

        loadDepIncrementalCache()

        for (name in this.depAnalysisEngine.finalDepMap.keys()) {
            verifyMultiplatformPost(this.depAnalysisEngine.finalDepMap[name].sourcePath, name, resolvedModuleMap)
                .inspect { v => verifiedModuleMap[name] = v }
                .inspectErr { errs =>
                    for (err in errs) { eprintln(err) }
                }.ok ?? return FAILED_BUILD_CHECK_RESULT
        }

        propagateFeatures()

        for (name in this.depAnalysisEngine.finalDepMap.keys()) {
            this.globalConfig.modulePath = this.depAnalysisEngine.finalDepMap[name].rootModulePath
            let (checkStatus, mockFlag) = this.moduleCheck(
                this.depAnalysisEngine.finalDepMap[name].sourcePath,
                this.depAnalysisEngine.finalDepMap[name].tomlInfo
            )
            checkFlag = checkFlag && checkStatus
            mockUsed = mockUsed || mockFlag
            if (!checkFlag) {
                return (checkFlag, mockUsed)
            }
        }

        if (COMMON_INFO.inWorkspace) {
            this.globalConfig.overrideCompileOption = COMMON_INFO.overrideCompileOption
            this.globalConfig.nativeOverrideOption = COMMON_INFO.nativeOverrideCompileOption
            this.globalConfig.rootName = WORKSPACE_NAME
        } else {
            this.globalConfig.overrideCompileOption =
                this.depAnalysisEngine.finalDepMap[this.globalConfig.rootName].tomlInfo.pkg.overrideCompileOption
        }

        // check env vars in override compile option
        let overrideEnvFlag = checkEnvVars(this.globalConfig.overrideCompileOption, this.globalConfig.rootPath) &&
            checkEnvVars(this.globalConfig.nativeOverrideOption, this.globalConfig.rootPath)
        if (overrideEnvFlag) {
            this.globalConfig.overrideCompileOption = replaceEnvVars(this.globalConfig.overrideCompileOption)
            this.globalConfig.nativeOverrideOption = replaceEnvVars(this.globalConfig.nativeOverrideOption)
        } else {
            return (overrideEnvFlag, mockUsed)
        }

        if (this.globalConfig.resolveType == RESOLVE_BUILD) {
            updateDepIncrementalCache()
        }

        return (this.replaceRealPkg(), mockUsed)
    }

    func verifyToml(tomlInfo: TomlInfo, path: String, curModulePath: String): Bool {
        var targetName: String = targetConfigName
        if (!crossCompileTarget.isEmpty()) {
            targetName = crossCompileTarget
            let nativeOverrideOption = tomlInfo.replaceNativeConfig(targetConfigName, this.globalConfig.isDebug)
            if (this.globalConfig.rootName == tomlInfo.pkg.name) {
                this.globalConfig.nativeOverrideOption = nativeOverrideOption
            }
        }
        if (!tomlInfo.replaceTargetConfig(path, targetName, this.globalConfig.isDebug, this.globalConfig.isTest)) {
            return false
        }
        if (COMMON_INFO.inWorkspace && COMMON_INFO.currentMembers.contains(getCanonicalPath(path)) &&
            !tomlInfo.replaceCommonInfo(this.globalConfig.rootPath, targetName)) {
            return false
        }

        if (this.globalConfig.isTest && !combineDevDependencies(tomlInfo)) {
            return false
        }

        if (!verifyModuleConfigPre(tomlInfo, curModulePath, this.globalConfig.rootPath,
            isTest: this.globalConfig.isTest, resolveMap: resolvedModuleMap)) {
            return false
        }

        return true
    }

    func isProjectCombineCheck(tomlInfo: TomlInfo): Bool {
        if (tomlInfo.pkg.packageConfiguration.contains(this.globalConfig.rootName) &&
            tomlInfo.pkg.packageConfiguration[this.globalConfig.rootName].combineAllDeps) {
            if (tomlInfo.profile.build.combined.isEmpty()) {
                eprintln("Error: field " +
                    "'package.package-configuration.${this.globalConfig.rootName}.combine-all-deps = true' " +
                    "is invalid when field 'profile.build.combined' is empty")
                return false
            }
            if (this.globalConfig.isForScript) {
                println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
                    "combine-all-deps module when depended by build.cj")
            } else if (this.globalConfig.isTest) {
                println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
                    "combine-all-deps module for test")
            } else {
                this.globalConfig.isProjectCombine = true
            }
        }
        return true
    }

    func depCheck(path: String, preName: String, prePath: String, requires: ?DepInfo): BuildCheckResult {
        // Check the path exists.
        if (!directoryExists(path)) {
            eprintln("Error: the path '${path}' does not exist")
            return FAILED_BUILD_CHECK_RESULT
        }

        let (tomlInfo, curModulePath) = this.getTomlAndPath(path, preName, requires) ??
            return FAILED_BUILD_CHECK_RESULT
        if (!tomlInfo.pkg.organization.isEmpty()) {
            tomlInfo.pkg.name = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
        }
        if (!this.verifyToml(tomlInfo, path, curModulePath)) {
            return FAILED_BUILD_CHECK_RESULT
        }
        if (!COMMON_INFO.inWorkspace && (this.globalConfig.rootName == tomlInfo.pkg.name)) {
            this.replaceMap = tomlInfo.replaceDeps

            // check central-repository replacement
            for ((name, replaceModule) in this.replaceMap) {
                if (let Some(version) <- replaceModule.version) {
                    let versionReq = RequireVersion(name, version)
                    if (!versionReq.isSingleVersion()) {
                        eprintln("Error: dependencies in 'replace' from central repository should be a single version, not a version range")
                        return FAILED_BUILD_CHECK_RESULT
                    }
                }
            }

            if (!isProjectCombineCheck(tomlInfo)) {
                return FAILED_BUILD_CHECK_RESULT
            }
            let (flag, _) = this.processDependencyModule(this.lockedMap, "", "", true)
            if (!flag) {
                return FAILED_BUILD_CHECK_RESULT
            }
        }

        if (!tomlInfo.profile.build.combined.isEmpty() && !this.globalConfig.isProjectCombine) {
            if (this.globalConfig.isForScript) {
                println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
                    "combined module when depended by build.cj")
            } else {
                for ((pkgName, outputType) in tomlInfo.profile.build.combined) {
                    this.superPkgMap[pkgName] = SuperPackageConfig(pkgName, outputType)
                }
            }
        }

        if (let Some(depInfo) <- requires) {
            if (let Some(outputType) <- depInfo.outputType) {
                tomlInfo.pkg.outputType = outputType
            }
            if (let Exe <- tomlInfo.pkg.outputType) {
                let pathLoc = Path(curModulePath).join(CONFIG_FILE_NAME).toString()
                tomlInfo.pkg.outputType = Static
            }
            // verify that `requires` block that added this module is compatible with its medatada
            if (preName != tomlInfo.pkg.name) {
                eprintln("Error: the require name '${preName}' is different to name '${tomlInfo.pkg.name}'")
                eprintln("  - ${Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()}")
                eprintln("  - ${Path(curModulePath).join(CONFIG_FILE_NAME).toString()}")
                return FAILED_BUILD_CHECK_RESULT
            }
        } else {
            let curDepInfo = DepInfo(getCanonicalPath(curModulePath))
            curDepInfo.outputType = tomlInfo.pkg.outputType
            if (this.depAnalysisEngine.finalDepMap.contains(tomlInfo.pkg.name)) {
                let (isAlternative, errInfo) = this.depAnalysisEngine.finalDepMap[tomlInfo.pkg.name].sourceInfo.isAlternative(curDepInfo)
                if (!errInfo.isEmpty()) {
                    eprintln("Error: modules with name '${tomlInfo.pkg.name}' in dependency tree are conflicted. " +
                        "If existing dependency was updated in cjpm.toml then do `cjpm update`")
                    eprintln(errInfo)
                    return FAILED_BUILD_CHECK_RESULT
                }
                if (!isAlternative) {
                    return (true, false)
                }
            }
            this.depAnalysisEngine.addDepInfo(tomlInfo.pkg.name, curDepInfo)
        }
        this.depAnalysisEngine.addDepPath(tomlInfo.pkg.name, curModulePath)
        this.depAnalysisEngine.addDepToml(tomlInfo.pkg.name, tomlInfo)
        this.depAnalysisEngine.addDepRoot(tomlInfo.pkg.name, this.globalConfig.modulePath)

        return this.processDependencyModule(tomlInfo.dependencies, curModulePath, tomlInfo.pkg.name, false)
    }

    // verify dependencies from central repository
    func repoDepCheck(finalDep: FinalDepModule): Bool {
        // lock is changed, print warning
        let finalVersion = finalDep.sourceInfo.version ?? return false
        if (let Some(lockedVersion) <- this.depAnalysisEngine.lockRepoDepMap.get(finalDep.name) &&
            (lockedVersion != finalVersion)) {
            eprintln("Warning: locked version '${lockedVersion}' of module '${finalDep.name}' " +
                "caused conflictions during dependency-analysis, use version '${finalVersion}' instead")
        }

        let tomlInfo = loadModuleFile(finalDep.sourcePath) ?? return false
        if (tomlInfo.isWorkSpaceConfig) { // always not
            eprintln("Error: dependency '${finalDep.name}' from central repository cannot be a workspace")
            return false
        }
        if (!tomlInfo.pkg.organization.isEmpty()) {
            tomlInfo.pkg.name = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
        }

        if ((let Some(outputType) <- finalDep.sourceInfo.outputType) && outputType.isValid) {
            tomlInfo.pkg.outputType = outputType
        }

        // verify tomlInfo
        if (!this.verifyToml(tomlInfo, finalDep.sourcePath, finalDep.sourcePath)) {
            return false
        }

        // configuration of project-level combine
        if (!tomlInfo.profile.build.combined.isEmpty() && !this.globalConfig.isProjectCombine) {
            if (this.globalConfig.isForScript) {
                println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as " +
                    "combined module when depended by build.cj")
            } else {
                for ((pkgName, outputType) in tomlInfo.profile.build.combined) {
                    this.superPkgMap[pkgName] = SuperPackageConfig(pkgName, outputType)
                }
            }
        }

        finalDep.tomlInfo = tomlInfo
        return true
    }

    func getTomlAndPath(path: String, preName: String, requires: ?DepInfo): Option<(TomlInfo, String)> {
        var tomlInfo = loadModuleFile(path) ?? return Option.None
        var curModulePath: String = path

        if (let Some(depInfo) <- requires) {
            if (tomlInfo.isWorkSpaceConfig) {
                let memPath = searchModuleFromWorkspace(tomlInfo, preName, curModulePath)
                if (memPath.isEmpty()) {
                    eprintln("Error: the member module '${preName}' does not exist in the workspace of '${path}'")
                    return Option.None
                }
                tomlInfo = loadModuleFile(memPath) ?? return Option.None
                depInfo.pathInWorkspace = memPath
                curModulePath = memPath
            }
        } else {
            if (this.globalConfig.isInstall && tomlInfo.isPkgConfig && tomlInfo.pkg.outputType != Exe) {
                eprintln("Error: output-type in ${Path(path).join(CONFIG_FILE_NAME).toString()} " +
                    "must be '${Exe}' for cjpm install")
                return Option.None
            }
            COMMON_INFO.moduleVersionMap.add(tomlInfo.pkg.name, tomlInfo.pkg.version)
            if (tomlInfo.pkg.organization.isEmpty()) {
                this.globalConfig.rootName = tomlInfo.pkg.name
            } else {
                this.globalConfig.rootName = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
            }
        }
        return (tomlInfo, curModulePath)
    }

    // requires - DepInfo that added current module as a dependency
    func moduleCheck(path: String, tomlInfo: TomlInfo): BuildCheckResult {
        if (!this.globalConfig.isSkipScript && (path != this.globalConfig.modulePath) &&
            (this.globalConfig.resolveType != RESOLVE_CHECK)) {
            if (fileExists(Path(path).join(BUILD_SCRIPT_NAME)) && !directoryExists(this.globalConfig.scriptDir)){
                if (!createDirectory(this.globalConfig.scriptDir)) {
                    return FAILED_BUILD_CHECK_RESULT
                }
            }
            let scriptConfig: ScriptConfig = makeScriptConfigByToml(this.globalConfig.scriptDir, path, tomlInfo,
                this.globalConfig.isDebug, this.globalConfig.isVerbose, this.globalConfig.isCross) ?? return FAILED_BUILD_CHECK_RESULT
            if (!runScript(scriptConfig, Pre, this.globalConfig.resolveType)) {
                return FAILED_BUILD_CHECK_RESULT
            }
            this.globalConfig.scriptList.add(scriptConfig)
        }
        if (COMMON_INFO.inWorkspace && !this.globalConfig.isSkipScript && (this.globalConfig.resolveType != RESOLVE_CHECK)) {
            if (fileExists(Path(path).join(BUILD_SCRIPT_NAME)) && !directoryExists(this.globalConfig.scriptDir)){
                if (!createDirectory(this.globalConfig.scriptDir)) {
                    return FAILED_BUILD_CHECK_RESULT
                }
            }
            let scriptConfig: ScriptConfig = makeScriptConfigByToml(this.globalConfig.scriptDir, path, tomlInfo,
                this.globalConfig.isDebug, this.globalConfig.isVerbose, this.globalConfig.isCross) ?? return FAILED_BUILD_CHECK_RESULT
            if (!runScript(scriptConfig, Pre, this.globalConfig.resolveType)) {
                return FAILED_BUILD_CHECK_RESULT
            }
            this.globalConfig.scriptList.add(scriptConfig)
        }

        if (!verifyModuleConfigPost(tomlInfo, path, isTest: this.globalConfig.isTest)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        if (!this.buildCheckTargetBinDep(tomlInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        this.isCrossBin = true
        if (!crossCompileTarget.isEmpty() && !this.buildCheckCrossBinDep(tomlInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }
        this.isCrossBin = false

        if (!this.buildCheckTargetFfi(tomlInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }
        if (!crossCompileTarget.isEmpty() && !this.buildCheckCrossFFi(tomlInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        let cjmpModuleInfo = verifiedModuleMap[tomlInfo.pkg.name]
        var deducer = FeatureDeducer(enabled: !this.globalConfig.noFeatureDeduce)
        deducer = deducer
            .addFeature(cjmpModuleInfo.alwaysEnabledFeatures)
            .setFeatureMapping(cjmpModuleInfo.mapping)

        let (actualSource, features) = constructSource(path, tomlInfo.pkg.srcDir.value, cjmpModuleInfo.sourceSets, deducer, this.globalConfig.verifiedEnabledFeatures, this.propagatedFeatures[tomlInfo.pkg.name])
            .inspectErr { err => eprintln("Error: ${err}") }
            .ok ?? return FAILED_BUILD_CHECK_RESULT

        tomlInfo.pkg.actualSources = actualSource
        tomlInfo.pkg.validPkgMap = getAllPkgs(actualSource, tomlInfo.pkg.name)

        let (processSuccess, mockUsedInDependentPackages) = this.processAllPkgs(tomlInfo, features, deducer)
        if (!processSuccess) {
            return (false, mockUsedInDependentPackages)
        }

        if (!this.processPackageConfiguration(tomlInfo.pkg)) {
            return (false, mockUsedInDependentPackages)
        }

        if (!this.checkTargetMacros(tomlInfo.targetConfigMap, tomlInfo.pkg)) {
            return (false, mockUsedInDependentPackages)
        }

        jsonMap[tomlInfo.pkg.name] = this.collectJsonInformation(tomlInfo)
        return (true, mockUsedInDependentPackages)
    }

    func buildCheckTargetBinDep(tomlInfo: TomlInfo): Bool {
        return buildCheckBinDependencies(tomlInfo, targetConfigName, this.binDepMap)
    }

    func buildCheckCrossBinDep(tomlInfo: TomlInfo): Bool {
        return buildCheckBinDependencies(tomlInfo, crossCompileTarget, this.crossBinDepMap)
    }
    func buildCheckTargetFfi(tomlInfo: TomlInfo): Bool {
        return buildCheckFfi(tomlInfo, targetConfigName, tomlInfo.ffi.cLibs, this.globalConfig.isDebug)
    }
    func buildCheckCrossFFi(tomlInfo: TomlInfo): Bool {
        return buildCheckFfi(tomlInfo, crossCompileTarget, tomlInfo.crossFfi.cLibs, this.globalConfig.isDebug)
    }

    func buildCheckBinDependencies(tomlInfo: TomlInfo, targetName: String, deps: HashMap<String, BinDeps>): Bool {
        let pkgMap = HashMap<String, RequiresPackagesInfo>() // Record all information in packageMap.
        let defaultSize = 16 // default parallel value
        let actualSize = min(defaultSize, maxParallelSize)

        if (tomlInfo.targetConfigMap.contains(targetName)) {
            let tomlPath: String = Path(tomlInfo.pkg.path).join(CONFIG_FILE_NAME).toString()
            var targetConfig = tomlInfo.targetConfigMap[targetName]
            if (!processCurModuleBinDep(pkgMap, targetConfig, tomlPath, actualSize)) {
                return false
            }
        }

        if (COMMON_INFO.inWorkspace && COMMON_INFO.currentMembers.contains(getCanonicalPath(tomlInfo.pkg.path)) &&
            COMMON_INFO.targetConfigMap.contains(targetName) && !COMMON_INFO.collectCjoPlatName.contains(targetName)) {
            let tomlPath: String = Path(this.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString()
            var targetConfig = COMMON_INFO.targetConfigMap[targetName]
            if (targetName == crossCompileTarget) {
                if (!processCurModuleBinDep(COMMON_INFO.crossPkgMap, targetConfig, tomlPath, actualSize)) {
                    return false
                }
            } else {
                if (!processCurModuleBinDep(COMMON_INFO.localPkgMap, targetConfig, tomlPath, actualSize)) {
                    return false
                }
            }
            COMMON_INFO.collectCjoPlatName.add(targetName)
        }

        if (COMMON_INFO.inWorkspace && COMMON_INFO.currentMembers.contains(getCanonicalPath(tomlInfo.pkg.path)) &&
            COMMON_INFO.targetConfigMap.contains(targetName)) {
            var commonMap: HashMap<String, RequiresPackagesInfo>
            if (targetName == crossCompileTarget) {
                commonMap = COMMON_INFO.crossPkgMap
            } else {
                commonMap = COMMON_INFO.localPkgMap
            }
            for ((fullName, v) in commonMap) {
                if (pkgMap.contains(fullName)) {
                    eprintln("Warning: the package '${fullName}' is imported multiple times, " +
                        "the path '${pkgMap[fullName].libPath}' is chosen")
                } else {
                    pkgMap[fullName] = v
                }
            }
        }

        this.binaryPkgList.add(all: pkgMap)

        let binDeps = packageReruiresSort(pkgMap)
        if (!binDeps.isEmpty()) {
            deps[tomlInfo.pkg.name] = binDeps
        }

        return true
    }

    func processCurModuleBinDep(pkgMap: HashMap<String, RequiresPackagesInfo>, targetConfig: TargetConfig,
        tomlPath: String, actualSize: Int64): Bool {
        // check debug/release target config
        if (this.globalConfig.isDebug) {
            if (!this.processPkgOption(pkgMap, targetConfig.debugTargetInfo.binDependencies.packageOption,
                actualSize, tomlPath)) {
                return false
            }
            if (!this.processPathOption(pkgMap, targetConfig.debugTargetInfo.binDependencies.pathOption,
                actualSize)) {
                return false
            }
        } else {
            if (!this.processPkgOption(pkgMap, targetConfig.releaseTargetInfo.binDependencies.packageOption,
                actualSize, tomlPath)) {
                return false
            }
            if (!this.processPathOption(pkgMap, targetConfig.releaseTargetInfo.binDependencies.pathOption,
                actualSize)) {
                return false
            }
        }
        // check common target config
        if (!this.processPkgOption(pkgMap, targetConfig.commonTargetInfo.binDependencies.packageOption,
            actualSize, tomlPath)) {
            return false
        }
        if (!this.processPathOption(pkgMap, targetConfig.commonTargetInfo.binDependencies.pathOption,
            actualSize)) {
            return false
        }
        return true
    }

    // process 'bin-dependencies.package-option' field in cjpm.toml.
    func processPkgOption(pkgMap: HashMap<String, RequiresPackagesInfo>, pkgOption: HashMap<String, String>,
        size: Int64, tomlPath: String): Bool {
        if (pkgOption.isEmpty()) {
            return true
        }

        let packageTasks = ArrayList<Future<Bool>>()
        let scanDepSem = Semaphore(size)

        for ((libName, cjoPath) in pkgOption) {
            scanDepSem.acquire()
            packageTasks.add(
                spawn {
                    =>
                    let flag: Bool = this.collectCjoInfo(cjoPath, pkgMap, false, libName, tomlPath)
                    scanDepSem.release()
                    return flag
                })
        }

        var taskFlag: Bool = true
        for (task in packageTasks) {
            if (!task.get()) {
                taskFlag = false
            }
        }
        return taskFlag
    }

    // process 'bin-dependencies.path-option' field in cjpm.toml.
    func processPathOption(pkgMap: HashMap<String, RequiresPackagesInfo>, pathOption: ArrayList<String>,
        size: Int64): Bool {
        if (pathOption.isEmpty()) {
            return true
        }

        let packageTasks = ArrayList<Future<Bool>>()
        let scanDepSem = Semaphore(size)

        let cjoPathSet = ArrayList<String>() // Get the path of all '.cjo' files.
        for (path in pathOption) {
            for (fileInfo in getFileList(path)) {
                let absPath: String = fileInfo.path.toString()
                if (absPath.endsWith(".cjo") && !absPath.endsWith("_chir.cjo")) {
                    cjoPathSet.add(absPath)
                }
            }
        }

        for (cjoPath in cjoPathSet) {
            scanDepSem.acquire()
            packageTasks.add(
                spawn {
                    =>
                    let flag: Bool = this.collectCjoInfo(cjoPath, pkgMap, true, "", "")
                    scanDepSem.release()
                    return flag
                })
        }

        var taskFlag: Bool = true
        for (task in packageTasks) {
            if (!task.get()) {
                taskFlag = false
            }
        }
        return taskFlag
    }

    func collectCjoInfo(cjoPath: String, pkgMap: HashMap<String, RequiresPackagesInfo>,
        isPathOption: Bool, libName: String, tomlPath: String): Bool {

        let cjoInfo = match (scanCjoInfo(cjoPath, this.globalConfig.isTest)) {
            case Some(data) => data
            case _ => return false
        }

        let (cangjiePath, libPath) = makePackagePath(cjoPath)
        let pkgInfo = RequiresPackagesInfo(cjoInfo.isMacroPackage, cangjiePath, libPath)
        if (isPathOption) {
            if (cjoInfo.isMacroPackage) {
                pkgInfo.libName = "-macro_${cjoInfo.packageName}"
            } else {
                pkgInfo.libName = cjoInfo.packageName
            }
            if (!isPackageExist(pkgInfo.libName, pkgInfo.libPath, false, "", pkgInfo.isMacroPackage) &&
                isSuperPackageExist(pkgInfo.libName, pkgInfo.libPath, this.isCrossBin)) {
                synchronized(this.packageMtx) {
                    this.superPkgSubMap.add(pkgInfo.libName, pkgInfo.libName.split(".")[0])
                }
            }
            pkgInfo.isDyLib = isDyLibExist(pkgInfo.libName, pkgInfo.libPath, this.isCrossBin)
        } else {
            pkgInfo.libName = libName
            if (!isPackageExist(pkgInfo.libName, libPath, true, "'bin-dependencies' field at ${tomlPath}",
                pkgInfo.isMacroPackage)) {
                return false
            }
            pkgInfo.isDyLib = isDyLibExist(pkgInfo.libName, libPath, this.isCrossBin)
        }
        for (k in cjoInfo.dependencies) {
            pkgInfo.requires.add(k.packageName)
        }
        for (k in cjoInfo.stdDependencies) {
            pkgInfo.stdRequires.add(k.packageName)
        }
        synchronized(this.packageMtx) {
            if (pkgMap.contains(cjoInfo.packageName)) {
                eprintln("Warning: the package '${cjoInfo.packageName}' is imported multiple times, " +
                    "the path '${pkgMap[cjoInfo.packageName].libPath}' is chosen")
            } else {
                pkgMap[cjoInfo.packageName] = pkgInfo
            }
        }
        return true
    }

    // Get package information by the '.cjo' file.
    func scanCjoInfo(cjoPath: String, isTest: Bool): ?RequiresPackages {
        let timeStamp = FileInfo(cjoPath).lastModificationTime.toUnixTimeStamp().toSeconds()
        if (this.binDepCacheMap.contains(cjoPath) && (timeStamp == this.binDepCacheMap[cjoPath].timeStamp)) {
            return this.binDepCacheMap[cjoPath]
        }

        let arguments = ArrayList<String>(["--scan-dependency", cjoPath])

        if (isTest) {
            arguments.add("--test")
        }

        var (scanFlag, outInfo, errInfo) = execWithOutput(COMPILE_TOOL, arguments)
        if (!scanFlag) {
            eprintln(errInfo)
            eprintln("Error: failed to obtain package import information of '${cjoPath}'")
            return None
        }

        var p = try {
            RequiresPackages.deserialize(DataModel.fromJson(JsonValue.fromStr(outInfo)))
        } catch (e: Exception) {
            eprintln(e.toString())
            eprintln("Error: failed to parse package import information of '${cjoPath}'")
            println(outInfo)
            return None
        }

        // CANGJIE_PATH check.
        let rootPkgName: String = swapOrgName(getRootPkgName(p.packageName))
        let (_, rootPath) = makePackagePath(cjoPath)
        if (!Path(rootPath).normalize().toString().endsWith(Path(rootPkgName).normalize().toString())) {
            eprintln("Error: the file '${cjoPath}' must be placed in the '${rootPkgName}' folder")
            return None
        }

        p.timeStamp = timeStamp

        synchronized (this.scanCjoMtx) {
            this.binDepCacheMap.add(cjoPath, p)
        }

        return p
    }

    func scanPkgInfo(pkgSource: CJMPSource, isTest!: Bool = false, compileOptions!: String = ""): Result<RequiresPackages, Array<ScanDependencyError>> {
        if (pkgSource.allPaths.size > 1) {
            let paths = pkgSource.allPaths
            return paths.iterator().allResult { it =>
                scanDependency(it, isTest: isTest, compileOptions: compileOptions).mapErr { e => [ e ] }
            }.flatMap { scans: Array<RequiresPackages> =>
                let name = scans[0].packageName
                let isMacro = scans[0].isMacroPackage
                let errors = ArrayList<String>()
                let anyDifferent = scans |> any { it => it.packageName != name || it.isMacroPackage != isMacro  }
                if (anyDifferent) {
                    errors.add("Multiplatform package contains different package headers.")
                    for ((i, scan) in scans.iterator().enumerate()) {
                        errors.add("  `${scan.packageName}` at `${paths[i]}`")
                    }
                }

                for ((i, source) in pkgSource._sources.iterator().enumerate()) {
                    let expectedFeatures = source.features
                    let expectedProduct = source.configuredProduct
                    let specifiedProduct = scans[i].product
                    let specifiedFeatures = scans[i].features
                    if (!(specifiedFeatures.contains(all: expectedFeatures)) ||
                        !(expectedFeatures.contains(all: specifiedFeatures))) {
                            let stringify = { it: Iterable<Feature> =>
                                if (let str <- (it |> collectString(delimiter: ", ")) && !str.isEmpty()) {
                                    str
                                } else { "<none>" }
                            }
                            errors.add(
                                "Feature header `${stringify(specifiedFeatures)}` of package `${name}` is different from expected `${stringify(expectedFeatures)}`"
                            )
                    }
                    if (expectedProduct != specifiedProduct) {
                        let errormesg = if (!expectedProduct) {
                            "Feature header of package `${name}` is missing `@NonProduct` annotation"
                        } else {
                            "Package `${name}` should be compiled as product, but feature header annotation `@NonProduct` is present in source code"
                        }
                        errors.add(errormesg)
                    }
                }

                if (errors.size != 0) {
                    return Err<RequiresPackages, Array<ScanDependencyError>>([MultiplatformMergeError(errors |> collectString(delimiter: '\n'))])
                } else {
                    let result = RequiresPackages()
                    result.packageName = name
                    result.accessLevel = scans[0].accessLevel
                    result.isMacroPackage = isMacro
                    result.mockUsed = scans |> any { it => it.mockUsed }
                    result.testUsed = scans |> any { it => it.testUsed }
                    result.dependencies = scans |> flatMap { it => it.dependencies.iterator() } |> collectHashSet |> collectArrayList
                    result.stdDependencies = scans |> flatMap { it => it.stdDependencies.iterator() } |> collectHashSet |> collectArrayList
                    result.features = HashSet() // Isn't necessary further
                    result.timeStamp = (scans.map { it => it.timeStamp } |> max) ?? 0
                    return Ok<RequiresPackages, Array<ScanDependencyError>>(result)
                }
            }
        }
        let pkgPath = pkgSource.allPaths[0]
        let pkgPathString = pkgPath.toString()
        let timeStamp = FileInfo(pkgPath).lastModificationTime.toUnixTimeStamp().toSeconds()
        if (!isTest && this.pkgBuildDepCacheMap.contains(pkgPathString) && (timeStamp == this.pkgBuildDepCacheMap[pkgPathString].timeStamp)) {
            return Ok(this.pkgBuildDepCacheMap[pkgPathString])
        } else if (isTest && this.pkgTestDepCacheMap.contains(pkgPathString) && (timeStamp == this.pkgTestDepCacheMap[pkgPathString].timeStamp)) {
            return Ok(this.pkgTestDepCacheMap[pkgPathString])
        }
        // update pkgDepCacheMap
        scanDependency(pkgPath, isTest: isTest, compileOptions: compileOptions).inspect { depRes =>
            var dep = depRes
            dep.timeStamp = timeStamp
            synchronized (this.scanPkgMtx) {
                if (isTest) {
                    this.pkgTestDepCacheMap.add(pkgPathString, dep)
                } else {
                    this.pkgBuildDepCacheMap.add(pkgPathString, dep)
                }
            }
        }.mapErr { e => [ e ] }
    }

    private func propagateFeatures(): Unit {
        for (name in this.depAnalysisEngine.finalDepMap.keys()) {
            propagatedFeatures[name] = HashSet()
        }

        let target = if (crossCompileTarget.isEmpty()) { targetConfigName } else { crossCompileTarget }
        for (name in this.depAnalysisEngine.finalDepMap.keys()) {
            let alwaysEnabledFeatures = verifiedModuleMap[name].alwaysEnabledFeatures
            let mapping = verifiedModuleMap[name].mapping
            let features = FeatureDeducer(enabled: !this.globalConfig.noFeatureDeduce)
                .addFeature(this.globalConfig.verifiedEnabledFeatures)
                .addFeature(alwaysEnabledFeatures)
                .deduceTargetFeatures(Triple.fromString(target).getOrThrow())
                .setFeatureMapping(mapping)
                .collect()

            propagatedFeatures[name].add(all: features)
            for (feature in features) {
                if (
                    let Some(fname) <- feature.depName &&
                    let Some(featureset) <- propagatedFeatures.get(fname)
                ) { featureset.add(feature) }
            }
        }
    }

    // read incremental cache of bin-dependencies and pkg-dependencies
    func loadDepIncrementalCache(): Unit {
        let depCachePath = Path(this.globalConfig.targetDir).join(DEP_INCREMENTAL_CACHE).toString()
        if (!fileExists(depCachePath)) {
            return
        }

        var buf = String.fromUtf8(File.readFrom(depCachePath))
        var dm = DataModel.fromJson(JsonValue.fromStr(buf))

        match (dm) {
            case data: DataModelStruct =>
                this.binDepCacheMap = HashMap<String, RequiresPackages>.deserialize(data.get("bin"))
                this.pkgBuildDepCacheMap = HashMap<String, RequiresPackages>.deserialize(data.get("pkg-build"))
                this.pkgTestDepCacheMap = HashMap<String, RequiresPackages>.deserialize(data.get("pkg-test"))

            case _ =>
                throw Exception("this data is not DataModelStruct")
                eprintln("Warning: load ${depCachePath} failed\n")
                return
        }
    }

    // update incremental cache of bin-dependencies and pkg-dependencies
    func updateDepIncrementalCache(): Unit {
        let binCachePath = Path(this.globalConfig.targetDir).join(DEP_INCREMENTAL_CACHE).toString()
        let dmCacheRoot = DataModelStruct()
            .add(field<HashMap<String, RequiresPackages>>("bin", this.binDepCacheMap))
            .add(field<HashMap<String, RequiresPackages>>("pkg-build", this.pkgBuildDepCacheMap))
            .add(field<HashMap<String, RequiresPackages>>("pkg-test", this.pkgTestDepCacheMap))
        let cacheString = dmCacheRoot.toJson().toJsonString()
        createAndWriteFile(binCachePath, cacheString)
        return
    }

    func processPackageConfiguration(pkg: Package): Bool {
        var checkFlag = true
        for ((fullName, v) in pkg.packageConfiguration) {
            if (!this.packageDepMap.contains(fullName)) {
                eprintln("Error: can not find the package '${fullName}' which is listed " +
                    "in 'package-configuration' field at ${getPath(pkg.path, CONFIG_FILE_NAME)}")
                checkFlag = false
                continue
            }

            if(!v.outputType.isValid) {
                if (let str <- v.outputType.toString() && !str.isEmpty()) {
                    eprintln("Error: the output-type '${str}' is invalid which is listed" +
                        " in 'package-configuration' field at ${getPath(pkg.path, CONFIG_FILE_NAME)}")
                    checkFlag = false
                }
            } else {
                this.packageDepMap[fullName].outputType = v.outputType
            }

            if (!v.compileOption.isEmpty()) {
                this.packageDepMap[fullName].compileOption += " ${v.compileOption}"
            }
        }

        return checkFlag
    }

    func checkTargetMacros(targetConfigMap: HashMap<String, TargetConfig>, pkg: Package): Bool {
        if (crossCompileTarget.isEmpty()) {
            return true
        }

        if (!targetConfigMap.contains(crossCompileTarget)) {
            return true
        }

        let allTargetMacros: String = targetConfigMap[crossCompileTarget].commonTargetInfo.allTargetMacros
        if (!allTargetMacros.isEmpty()) {
            if (allTargetMacros != "all") {
                eprintln("Error: the 'target.${crossCompileTarget}.${COMPILE_TARGET_MACROS}' field " +
                    "'${allTargetMacros}' in ${getPath(pkg.path, CONFIG_FILE_NAME)} is wrong, optional value is 'all'")
                return false
            }
            this.targetMacroMods.add(pkg.name)
            return true
        }

        let someTargetMacros = targetConfigMap[crossCompileTarget].commonTargetInfo.someTargetMacros
        if (!someTargetMacros.isEmpty()) {
            var checkFlag = true
            for (fullName in someTargetMacros) {
                if (!this.packageDepMap.contains(fullName)) {
                    eprintln("Error: can not find the package '${fullName}' which is listed in " +
                        "'${COMPILE_TARGET_MACROS}' field at ${getPath(pkg.path, CONFIG_FILE_NAME)}")
                    checkFlag = false
                    continue
                }
                if (!this.packageDepMap[fullName].isMacroPackage) {
                    eprintln("Error: the package '${fullName}' in " +
                        "'target.${crossCompileTarget}.${COMPILE_TARGET_MACROS}' field in " +
                        "${getPath(pkg.path, CONFIG_FILE_NAME)} is wrong, optional value must be a macro package name")
                    checkFlag = false
                    continue
                }
                this.targetMacroPkgs.add(fullName)
            }
            return checkFlag
        }

        return true
    }

    // Processing dependency module.
    func processDependencyModule(requires: HashMap<String, DepInfo>, prePath: String, preName: String, isLocked: Bool): BuildCheckResult {
        var buildCheckFlag = true
        var mockUsed = false
        for ((name, v) in requires) {
            // convert to absolute path for future comparisons
            if (let Some(vpath) <- v.path) {
                v.path = getPath(prePath, vpath)
            }

            if (isLocked) {
                v.importedPath = LOCK_FILE_NAME
                v.importedModuleName = LOCK_FILE_NAME
            } else {
                v.importedPath = Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()
                v.importedModuleName = preName
            }

            var finalDepInfo: DepInfo = v
            if (let Some(version) <- finalDepInfo.version) {
                // continue if replaced
                if (this.replaceMap.contains(name)) {
                    finalDepInfo = this.replaceMap[name]
                    if (let Some(newVersion) <- finalDepInfo.version) {
                        this.depAnalysisEngine.repoDepList.add((name, finalDepInfo))
                        continue
                    }
                } else {
                    this.depAnalysisEngine.repoDepList.add((name, finalDepInfo))
                    continue
                }
            }

            if (let Some(currentFinal) <- this.depAnalysisEngine.finalDepMap.get(name)) {
                let current = currentFinal.sourceInfo
                if (this.replaceMap.contains(name)) {
                    continue
                }
                let (isAlternative, errInfo) = current.isAlternative(v)
                let isGitLocalDep = this.checkGitLocalDep(name, current, v)
                if (!errInfo.isEmpty() && !isGitLocalDep) {
                    eprintln("Error: modules with name '${name}' in dependency tree are conflicted. " +
                        "If existing dependency was updated in cjpm.toml then do `cjpm update`")
                    eprintln(errInfo)
                    buildCheckFlag = false
                    continue
                }
                if (!errInfo.isEmpty() && isGitLocalDep) {
                    continue
                }
                if (errInfo.isEmpty() && !isAlternative) {
                    current.updateWith(v)
                    continue
                }
            } else if (let Some(replaceDepInfo) <- this.replaceMap.get(name)) {
                finalDepInfo = replaceDepInfo
                if (let Some(version) <- finalDepInfo.version) {
                    // replace with a central-repository dependence
                    this.depAnalysisEngine.repoDepList.add((name, finalDepInfo))
                    continue
                }
            }

            this.depAnalysisEngine.addDepInfo(name, finalDepInfo)

            let depInfo: DepInfo = this.depAnalysisEngine.finalDepMap[name].sourceInfo
            let (flag, isOnlineDep, depPath) = getDepPath(name, depInfo, prePath, this.globalConfig)
            if (!flag) {
                buildCheckFlag = false
                continue
            }

            freshGitPathDep(depInfo, UPDATE_GLOBAL_CONFIG)

            // Recursive.
            let (checkSuccessful, mockUsedInDependentPackage) = this.depCheck(depPath, name, prePath, depInfo)
            if (isOnlineDep && !this.globalConfig.gitStack.pop()) {
                eprintln("Error: failed to analyse online dependencies")
                buildCheckFlag = false
                continue
            }

            if (!checkSuccessful) {
                buildCheckFlag = false
            }
            if (mockUsedInDependentPackage) {
                mockUsed = true
            }
        }

        return (buildCheckFlag, mockUsed)
    }

    // Get valid package by validPackagePath.
    func processAllPkgs(tomlInfo: TomlInfo, features: HashSet<Feature>, deducer: FeatureDeducer): BuildCheckResult {
        var packageTasks = ArrayList<Future<(Bool, Bool)>>()
        let defaultSize = 16 // default parallel value
        let actualSize = min(defaultSize, maxParallelSize)
        let scanDepSem = Semaphore(actualSize)

        let directDeps = tomlInfo.getDirectDependencies()
        if (this.binDepMap.contains(tomlInfo.pkg.name)) {
            for (fullName in this.binDepMap[tomlInfo.pkg.name].packageRequiresSort) {
                let rootPkgName: String = getRootPkgName(fullName)
                if (!rootPkgName.isEmpty()) {
                    directDeps.add(rootPkgName)
                }
            }
        }
        if (this.crossBinDepMap.contains(tomlInfo.pkg.name)) {
            for (fullName in this.crossBinDepMap[tomlInfo.pkg.name].packageRequiresSort) {
                let rootPkgName: String = getRootPkgName(fullName)
                if (!rootPkgName.isEmpty()) {
                    directDeps.add(rootPkgName)
                }
            }
        }

        for (pkg in tomlInfo.pkg.validPkgMap) {
            if (!this.globalConfig.isTest && pkg.hasTestFiles && !pkg.hasProdFiles) {
                continue
            }
            scanDepSem.acquire()
            packageTasks.add(
                spawn {
                    =>
                    let pkgInfo: ResolveItem = ResolveItem(tomlInfo)
                    pkgInfo.packagePath = pkg.sources
                    pkgInfo.hasProdFiles = pkg.hasProdFiles
                    pkgInfo.hasTestFiles = pkg.hasTestFiles
                    pkgInfo.features = features
                    pkgInfo.featureDeducer = deducer
                    let (flag, mockUsed) = runPackageTask(pkgInfo, pkg.hashCode, pkg.timeStamp, directDeps)
                    scanDepSem.release()
                    return (flag, mockUsed)
                })
        }

        var checkSuccessful = true
        var mockUsed = false
        for (task in packageTasks) {
            let (packageCheckSuccessful, mockUsedInPackage) = task.get()
            if (!packageCheckSuccessful) {
                checkSuccessful = false
            }
            if (mockUsedInPackage) {
                mockUsed = true
            }
        }

        return (checkSuccessful, mockUsed)
    }

    func runPackageTask(pkgInfo: ResolveItem, fileNameHash: String, timeStamp: Int64,
        directDeps: HashSet<String>): BuildCheckResult {

        let info = getPackageInfo(pkgInfo.packagePath, pkgInfo.compileOption, pkgInfo.customizedOption) ??
            return FAILED_BUILD_CHECK_RESULT

        pkgInfo.fullName = info.packageName
        pkgInfo.isMacroPackage = info.isMacroPackage
        pkgInfo.accessLevel = info.accessLevel
        if (!pkgInfo.isPureTestPkg()) {
            pkgInfo.hasTestsInProdFiles = info.testUsed
        }

        if (!packageCheck(pkgInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }
        if (!appendPkgInfo(pkgInfo, fileNameHash, timeStamp)) {
            synchronized(this.errPrintMtx) {
                eprintln("Error: different package with same name '${pkgInfo.fullName}' is not supported")
            }
            return FAILED_BUILD_CHECK_RESULT
        }

        for (d in info.dependencies) {
            let depRootPkgName: String = getRootPkgName(d.packageName)
            if (!directDeps.contains(depRootPkgName)) {
                synchronized(this.errPrintMtx) {
                    eprintln("Error: root package '${pkgInfo.rootPkgName}' imports package '${d.packageName}' " +
                        "in its source code, but it is not added as a dependency in cjpm.toml")
                }
                return FAILED_BUILD_CHECK_RESULT
            }
            pkgInfo.requireDoublePkgs.add((d.packageName, d.orPkgName))
        }

        for (sd in info.stdDependencies) {
            pkgInfo.stdRequires.add(sd.packageName)
        }

        // The output type of current module is executable, but other package should be static in this module.
        if (pkgInfo.modulePath == this.globalConfig.modulePath && isSubpackage(pkgInfo.fullName)) {
            if (pkgInfo.outputType == Exe) {
                pkgInfo.outputType = Static
            }
        }

        if (pkgInfo.isMacroPackage) {
            // The output type of macro package should be 'dynamic'.
            if (let Static <- pkgInfo.outputType) {
                pkgInfo.outputType = Dynamic
            }
        }

        synchronized(this.packageMtx) {
            this.packageDepMap[pkgInfo.fullName] = pkgInfo
        }
        return (true, info.mockUsed)
    }

    // check if dep-package is executable
    func checkExePackage(src: String, dep: String): Bool {
        let rootPkgName: String = dep.split(".")[0]
        if (!this.depAnalysisEngine.finalDepMap.contains(rootPkgName)) {
            // bin-dependencies
            return false
        }
        let tomlInfo: TomlInfo = this.depAnalysisEngine.finalDepMap[rootPkgName].tomlInfo
        var flag: Bool = false
        if (dep == rootPkgName) {
            flag = (tomlInfo.pkg.outputType == Exe)
        }
        if (tomlInfo.pkg.packageConfiguration.contains(dep)) {
            flag = (tomlInfo.pkg.packageConfiguration[dep].outputType == Exe)
        }

        if (flag) {
            synchronized(this.errPrintMtx) {
                eprintln("Error: package '${src}' imports root package '${dep}' " +
                    "in its source code, but root package is not compiled as a lib (outputType == 'executable')")
            }
        }
        return flag
    }

    // consistency check between package name and path.
    func packageCheck(pkgInfo: ResolveItem): Bool {
        var isValid = true
        for (source in pkgInfo.packagePath._sources) {
            let canonicalizedPath = getPath(pkgInfo.modulePath, source.sourceSetDir.toString())
            let packagePath = source.srcDir.toString()
            let pkgPath = packagePath.removePrefix(canonicalizedPath)
            let actualName = getActualName(pkgPath, pkgInfo.rootPkgName)
            if (pkgInfo.fullName != actualName) {
                synchronized (this.errPrintMtx) {
                    eprintln("Error: the package name in ${packagePath} is wrong, the right name should be '${actualName}'")
                }
                isValid = false
            }
        }
        return isValid
    }

    func appendPkgInfo(pkgInfo: ResolveItem, fileNameHash: String, timeStamp: Int64): Bool {
        synchronized(this.packageMtx) {
            if (this.sourceCodePkgList.contains(pkgInfo.fullName)) {
                return false
            }
            this.sourceCodePkgList.add(pkgInfo.fullName)
            this.pkgCacheMap[pkgInfo.fullName] = CacheItem(pkgInfo.rootPkgName, fileNameHash, timeStamp)
        }
        return true
    }

    func getPackageInfo(pkgSource: CJMPSource, cmdOption: String, customizedMap: HashMap<String, String>): ?RequiresPackages {
        let errors = ArrayList<String>()
        for (path in pkgSource.allPaths) {
            if (!sourcePathCheck(path)) {
                errors.add("Error: the '${path}' contains illegal characters")
            }
        }

        if (!errors.isEmpty()) {
            synchronized (this.errPrintMtx) {
                eprintln(
                    String.join(errors |> collectArray, delimiter: '\n')
                )
            }
            return None
        }

        var options = ""
        if (cmdOption.contains("--cfg")) {
            options = cmdOption
        }
        for (customizedKey in this.globalConfig.customizedOption) {
            if (customizedMap.contains(customizedKey) && customizedMap[customizedKey].contains("--cfg")) {
                options += " ${customizedMap[customizedKey]}"
            }
        }

        scanPkgInfo(pkgSource, isTest: this.globalConfig.isTest, compileOptions: options)
            .inspectErr { errs => synchronized (this.errPrintMtx) {
                for (e in errs) { eprintln(e) }
            } }.ok
    }

    func collectJsonInformation(tomlInfo: TomlInfo): ArrayList<String> {
        var jsonData = ArrayList<String>()
        jsonData.add(tomlInfo.pkg.name)
        for ((k, v) in tomlInfo.ffi.cLibs) {
            jsonData.add(k)
            jsonData.add(v)
        }
        for ((k, v) in tomlInfo.crossFfi.cLibs) {
            jsonData.add(k)
            jsonData.add(v)
        }
        jsonData.add(tomlInfo.pkg.outputType.toString())
        jsonData.add(tomlInfo.pkg.compileOption)
        jsonData.add(tomlInfo.pkg.linkOption)
        for ((k, v) in tomlInfo.profile.customizedOption) {
            jsonData.add(k)
            jsonData.add(v)
        }
        for ((k, v) in tomlInfo.pkg.packageConfiguration) {
            jsonData.add(k)
            jsonData.add(v.outputType.toString())
            jsonData.add(v.compileOption)
        }
        jsonData.add(tomlInfo.pkg.overrideCompileOption)
        if (this.binDepMap.contains(tomlInfo.pkg.name)) {
            let deps = this.binDepMap[tomlInfo.pkg.name]
            for (fullName in deps.packageRequiresSort) {
                jsonData.add(deps.packageRequires[fullName].libPath)
                jsonData.add(deps.packageRequires[fullName].libName)
            }
        }
        if (this.crossBinDepMap.contains(tomlInfo.pkg.name)) {
            let deps = this.crossBinDepMap[tomlInfo.pkg.name]
            for (fullName in deps.packageRequiresSort) {
                jsonData.add(deps.packageRequires[fullName].libPath)
                jsonData.add(deps.packageRequires[fullName].libName)
            }
        }
        return jsonData
    }

    func replaceRealPkg(): Bool {
        for ((sourcePkgName, item) in this.packageDepMap) {
            var depPkgName = ""
            for ((pkgName, orPkgName) in item.requireDoublePkgs) {
                if (orPkgName.isEmpty() || (!this.sourceCodePkgList.contains(orPkgName) &&
                    !this.binaryPkgList.contains(orPkgName))) {
                    depPkgName = pkgName
                } else {
                    depPkgName = orPkgName
                }
                if (checkExePackage(sourcePkgName, depPkgName)) {
                    return false
                }
                item.requires.add(depPkgName)
            }
        }
        return true
    }

    func checkSuperPkgDep(): Unit {
        for ((superPkgName, superPkgCfg) in this.superPkgMap) {
            // add requirement between combined root packages
            if (!this.packageDepMap.contains(superPkgName)) {
                continue
            }
            let checkedPkgSet: HashSet<String> = HashSet<String>()
            let extraSuperPkgDep: HashSet<String> = HashSet<String>()
            for (subPkg in superPkgCfg.subPkgSet) {
                extraSuperPkgDep.clear()
                let extraSuperPkgDep: HashSet<String> = HashSet<String>()
                getDepBetweenSuperPkg(subPkg, extraSuperPkgDep, checkedPkgSet)
                this.packageDepMap[superPkgName].requires.add(all: extraSuperPkgDep)
            }

            // add combine deps of all sub-pkg deps of super package from combined modules
            // and clear all these sub-pkg deps
            extraSuperPkgDep.clear()
            checkedPkgSet.remove(superPkgName)
            getDepBetweenSuperPkg(superPkgName, extraSuperPkgDep, checkedPkgSet)
            this.packageDepMap[superPkgName].requires.add(all: extraSuperPkgDep)
            this.packageDepMap[superPkgName].requires.remove(superPkgName)

            let dupSubPkgDep: HashSet<String> = HashSet<String>()
            for (dep in this.packageDepMap[superPkgName].requires) {
                if ((dep.split(".").size > 1) && (dep.split(".")[0] != superPkgName) &&
                    this.superPkgMap.contains(dep.split(".")[0])) {
                    dupSubPkgDep.add(dep)
                }
            }
            this.packageDepMap[superPkgName].requires.remove(all: dupSubPkgDep)

            // add force-linked sub-packages into requires, and record them
            // after toposort, remove them from requires
            this.packageDepMap[superPkgName].forcedRequires.add(all: superPkgCfg.subPkgSet)
            this.packageDepMap[superPkgName].forcedRequires.remove(all: this.packageDepMap[superPkgName].requires)
            this.packageDepMap[superPkgName].requires.add(all: this.packageDepMap[superPkgName].forcedRequires)
        }
    }

    func getDepBetweenSuperPkg(pkgName: String, extraSuperPkgDep: HashSet<String>,
        checkedPkgSet: HashSet<String>): Unit {
        if (checkedPkgSet.contains(pkgName)) {
            return
        }
        checkedPkgSet.add(pkgName)
        if (!this.packageDepMap.contains(pkgName)) {
            return
        }
        let srcSuperPkgName = pkgName.split(".")[0]
        for (dep in this.packageDepMap[pkgName].requires) {
            let dstSuperPkgName = dep.split(".")[0]
            if (srcSuperPkgName != dstSuperPkgName && this.superPkgMap.contains(dstSuperPkgName)) {
                extraSuperPkgDep.add(dstSuperPkgName)
            }
            getDepBetweenSuperPkg(dep, extraSuperPkgDep, checkedPkgSet)
        }
    }

    func checkGitLocalDep(name: String, curInfo: DepInfo, newInfo: DepInfo): Bool {
        if (!(curInfo.isGitDep() && newInfo.isPathDep()) || !this.depAnalysisEngine.finalDepMap.contains(name)) {
            return false
        }
        return this.depAnalysisEngine.finalDepMap[name].sourcePath == newInfo.path.getOrDefault({=> ""})
    }
}

func getActualName(pkgPath: String, rootPkgName: String): String {
    var actualName: String = rootPkgName
    if (pkgPath.isEmpty()) {
        return actualName
    }

    let dirs: Array<String> = pkgPath.removePrefix(PATH_SPLIT_CHAR).split(PATH_SPLIT_CHAR)
    for (dir in dirs) {
        let flag: Bool = dir.contains("-") || dir.contains(" ") || dir.contains(".")
        if (flag) {
            actualName += ".`${dir}`"
        } else {
            actualName += ".${dir}"
        }
    }

    return actualName
}

func constructSource(modulePath: String, srcDir: String, sourceSets: SourceLocations, deducer: FeatureDeducer, enabledFeatures: Array<Feature>, _globalFeatures: HashSet<Feature>): Result<(CJMPSource, HashSet<Feature>), String> {
    var _deducer = deducer
    let target = if (crossCompileTarget.isEmpty()) { targetConfigName } else { crossCompileTarget }
    if (let Some(triple) <- Triple.fromString(target).ok) {
        _deducer = _deducer.deduceTargetFeatures(triple)
    }
    let localFeatures = _deducer
        .addFeature(enabledFeatures)
        .collect()
    let globalFeatures = _deducer
        .addFeature(_globalFeatures)
        .collect()
    assertion { globalFeatures.contains(all: localFeatures) }
    if (sourceSets.isEmpty) {
        return Ok((CJMPSource(modulePath, srcDir), localFeatures))
    } else {
        return sourceSets.selectSourceSets(modulePath, localFeatures, globalFeatures).map { it => (it, localFeatures) }
    }
}

func getAllPkgs(srcDir: CJMPSource, rootPkgName: String): Array<PackageInfo> {
    var pkgDirQueue = [srcDir]
    var validPkgs = ArrayList<PackageInfo>()
    var pathDepth = 0
    do {
        pathDepth++
        var temp = Array<CJMPSource>().iterator()
        for (pkgDir in pkgDirQueue) {
            let p = initPackageInfo(pkgDir)
                .inspectErr { e => println(e) }
                .ok ?? continue
            validPkgs.add(p)
            temp = temp |> concat(pkgDir.dirs())
        }
        pkgDirQueue = temp |> collectArray
    } while (pkgDirQueue.size != 0)

    if (pathDepth > 20) {
        eprintln("Warning: the module depth of the '${rootPkgName}' is more than 20")
    }
    return validPkgs.toArray()
}

func initPackageInfo(pkgDir: CJMPSource): Result<PackageInfo, String> {
    var hasTestFile = false
    var hasProdFile = false
    var fileList = ArrayList<String>()
    var timeStamp: Int64 = Int64.Min
    for (fileInfo in pkgDir.files()) {
        let path = canonicalize(fileInfo.path)
        if (path.fileName.endsWith("_test.cj")) {
            hasTestFile = true
            fileList.add(path.toString())
            timeStamp = max(timeStamp, fileInfo.lastModificationTime.toUnixTimeStamp().toSeconds())
        } else if (path.extensionName == "cj") {
            hasProdFile = true
            fileList.add(path.toString())
            timeStamp = max(timeStamp, fileInfo.lastModificationTime.toUnixTimeStamp().toSeconds())
        }
    }
    if (hasTestFile || hasProdFile) {
        sort(fileList, by: cmpStr, stable: true)
        return Ok(PackageInfo(
            pkgDir, bufferHashCode(fileList),
            timeStamp, hasTestFile, hasProdFile
        ))
    }
    if (pkgDir.allPaths.size == 1) {
        return Err("Warning: there is no '.cj' file in directory '${pkgDir.allPaths[0]}', and its subdirectories will not be scanned as source code")
    } else {
        return Err("Warning: there is no '.cj' file in source-set directories '${pkgDir.allPaths}', and its subdirectories will not be scanned as source code")
    }
}

enum ScanDependencyError <: ToString {
    | FailedToExecute(String, String)
    | FailedToParse(String, String)
    | MultiplatformMergeError(String)

    public func toString(): String {
        match (this) {
            case FailedToExecute(cmd, error) =>
                let sb = StringBuilder()
                sb.append(error)
                sb.append("\n")
                sb.append("Error: failed to obtain package import information, please execute '${cmd}' to check")
                sb.toString()
            case FailedToParse(cmd, out) =>
                let sb = StringBuilder()
                sb.append("Error: failed to parse package import information, please execute '${cmd}' to check")
                sb.append("\n")
                sb.append(out)
                sb.toString()
            case MultiplatformMergeError(str) => str
        }
    }
}

func scanDependency(packagePath: Path, isTest!: Bool = false, compileOptions!: String = ""): Result<RequiresPackages, ScanDependencyError> {
    let arguments = ArrayList<String>()
    arguments.add(all: ["-p", "${packagePath}"])
    arguments.add("--scan-dependency")

    if (isTest) {
        arguments.add("--test")
    }
    arguments.add(all: extractCfgOption(compileOptions))

    let cmd: String = COMPILE_TOOL + " " + String.join(arguments.toArray(), delimiter: " ")
    let (scanFlag, out, error) = execWithOutput(COMPILE_TOOL, arguments)

    if (!scanFlag) {
        return Err(FailedToExecute(cmd, error))
    }

    try {
        Ok(RequiresPackages.deserialize(DataModel.fromJson(JsonValue.fromStr(out))))
    } catch (e: GetResultException) { // The features, specified in package are incorrect
        let sb = StringBuilder()
        sb.append(e.message.trimAscii())
        sb.append("\n\n")
        sb.append(out)
        Err(FailedToParse(cmd, sb.toString()))
    } catch (_: Exception) {
        Err(FailedToParse(cmd, out))
    }
}
