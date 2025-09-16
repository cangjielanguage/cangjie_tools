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

// left - whether the build check is successful
// right - whether mocks are used
type BuildCheckResult = (Bool, Bool)

let FAILED_BUILD_CHECK_RESULT = (false, false)

class DepModel {
    // Dep model config
    public var globalConfig: GlobalConfig = GlobalConfig()

    // All module dependency information for the current project.
    public var depMap: HashMap<String, DepInfo> = HashMap<String, DepInfo>()
    public var depRootMap: HashMap<String, String> = HashMap<String, String>()
    public var depPathMap: HashMap<String, String> = HashMap<String, String>()
    public var depTomlMap: HashMap<String, TomlInfo> = HashMap<String, TomlInfo>()
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
    // Collect zero indegree packages
    var readyPkgs: ArrayList<String> = ArrayList<String>()
    public var targetMacroPkgs: HashSet<String> = HashSet<String>() // used by cross compile
    public var targetMacroMods: HashSet<String> = HashSet<String>() // used by cross compile

    // Super package record
    public var superPkgMap: HashMap<String, SuperPackageConfig> = HashMap<String, SuperPackageConfig>()
    // Sub package map to super package
    public var superPkgSubMap: HashMap<String, String> = HashMap<String, String>()
    public var isCrossBin = false
    // Binary dependencies record
    public var binDepCacheMap: HashMap<String, RequiresPackages> = HashMap<String, RequiresPackages>()

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

    public func resolve(path: String, locked: HashMap<String, DepInfo>): BuildCheckResult {
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
            let (flag, _) = this.processDependencyModule(locked, "", true)
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
            this.globalConfig.modulePath = member
            let (checkStatus, mockFlag) = this.depCheck(member, "", "", None)
            checkFlag = checkFlag && checkStatus
            mockUsed = mockUsed || mockFlag
        }
        if (!checkFlag) {
            return (checkFlag, mockUsed)
        }

        loadBinIncrementalCache()

        for (name in this.depMap.keys()) {
            this.globalConfig.modulePath = this.depRootMap[name]
            let (checkStatus, mockFlag) = this.moduleCheck(this.depPathMap[name], this.depTomlMap[name])
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
                this.depTomlMap[this.globalConfig.rootName].pkg.overrideCompileOption
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
            updateBinIncrementalCache()
        }

        return (this.replaceRealPkg(), mockUsed)
    }

    func verifyToml(tomlInfo: TomlInfo, path: String, curModulePath: String): BuildCheckResult {
        var targetName: String = targetConfigName
        if (!crossCompileTarget.isEmpty()) {
            targetName = crossCompileTarget
            let nativeOverrideOption = tomlInfo.replaceNativeConfig(targetConfigName, this.globalConfig.isDebug)
            if (this.globalConfig.rootName == tomlInfo.pkg.name) {
                this.globalConfig.nativeOverrideOption = nativeOverrideOption
            }
        }
        if (!tomlInfo.replaceTargetConfig(path, targetName, this.globalConfig.isDebug, this.globalConfig.isTest)) {
            return FAILED_BUILD_CHECK_RESULT
        }
        if (COMMON_INFO.inWorkspace && COMMON_INFO.currentMembers.contains(getCanonicalPath(path)) &&
            !tomlInfo.replaceCommonInfo(this.globalConfig.rootPath, targetName)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        if (this.globalConfig.isTest && !combineDevDependencies(tomlInfo)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        if (!verifyModuleConfigPre(tomlInfo, curModulePath, this.globalConfig.rootPath,
            isTest: this.globalConfig.isTest)) {
            return FAILED_BUILD_CHECK_RESULT
        }

        return (true, false)
    }

    func depCheck(path: String, preName: String, prePath: String, requires: ?DepInfo): BuildCheckResult {
        // Check the path exists.
        if (!directoryExists(path)) {
            eprintln("Error: the path '${path}' does not exist")
            return FAILED_BUILD_CHECK_RESULT
        }

        let (tomlInfo, curModulePath) = this.getTomlAndPath(path, preName, requires) ??
            return FAILED_BUILD_CHECK_RESULT
        let (flag, _) = this.verifyToml(tomlInfo, path, curModulePath)
        if (!flag) {
            return FAILED_BUILD_CHECK_RESULT
        }
        if (!COMMON_INFO.inWorkspace && (this.globalConfig.rootName == tomlInfo.pkg.name)) {
            this.replaceMap = tomlInfo.replaceDeps
            let (flag, _) = this.processDependencyModule(this.lockedMap, "", true)
            if (!flag) {
                return FAILED_BUILD_CHECK_RESULT
            }
        }

        if (!tomlInfo.profile.build.combined.isEmpty()) {
            if (this.globalConfig.isForScript) {
                println("Warning: module '${tomlInfo.pkg.name}' will not be compiled as combined module when depended by build.cj")
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
            if (tomlInfo.pkg.outputType == EXE_TYPE) {
                let pathLoc = Path(curModulePath).join(CONFIG_FILE_NAME).toString()
                eprintln("Error: the 'output-type' field of dependent module can't be '${EXE_TYPE}' in ${pathLoc}")
                return FAILED_BUILD_CHECK_RESULT
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
            if (this.depMap.contains(tomlInfo.pkg.name)) {
                let (isAlternative, errInfo) = this.depMap[tomlInfo.pkg.name].isAlternative(curDepInfo)
                if (!errInfo.isEmpty()) {
                    eprintln("Error: modules with name '${tomlInfo.pkg.name}' in dependency tree are conflicted. " +
                        "If existing dependency was updated in cjpm.toml then do `cjpm update`")
                    eprintln(errInfo)
                    return FAILED_BUILD_CHECK_RESULT
                } else if (!isAlternative) {
                    return (true, false)
                }
            }
            this.depMap[tomlInfo.pkg.name] = curDepInfo
        }
        this.depPathMap[tomlInfo.pkg.name] = curModulePath
        this.depTomlMap[tomlInfo.pkg.name] = tomlInfo
        this.depRootMap[tomlInfo.pkg.name] = this.globalConfig.modulePath

        return this.processDependencyModule(tomlInfo.dependencies, curModulePath, false)
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
            if (this.globalConfig.isInstall && tomlInfo.isPkgConfig && tomlInfo.pkg.outputType != EXE_TYPE) {
                eprintln("Error: output-type in ${Path(path).join(CONFIG_FILE_NAME).toString()} " +
                    "must be 'executable' for cjpm install")
                return Option.None
            }
            COMMON_INFO.moduleVersionMap.add(tomlInfo.pkg.name, tomlInfo.pkg.version)
            this.globalConfig.rootName = tomlInfo.pkg.name
        }
        return (tomlInfo, curModulePath)
    }

    // requires - DepInfo that added current module as a dependency
    func moduleCheck(path: String, tomlInfo: TomlInfo): BuildCheckResult {
        if (!this.globalConfig.isSkipScript && (path != this.globalConfig.modulePath) &&
            (this.globalConfig.resolveType != RESOLVE_CHECK)) {
            var scriptConfig: ScriptConfig = makeScriptConfigByToml(this.globalConfig.scriptDir, path, tomlInfo,
                this.globalConfig.isDebug, this.globalConfig.isVerbose, this.globalConfig.isCross) ?? return FAILED_BUILD_CHECK_RESULT
            if (!runScript(scriptConfig, BUILD_SCRIPT_STEP_PRE, this.globalConfig.resolveType)) {
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

        let validPkgs = getAllPkgs(tomlInfo.pkg.srcDir, tomlInfo.pkg.name) ?? return FAILED_BUILD_CHECK_RESULT
        tomlInfo.pkg.validPkgMap = validPkgs
        let (processSuccess, mockUsedInDependentPackages) = this.processAllPkgs(tomlInfo)
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

    func collectCjoInfo(cjoPath: String, pkgMap: HashMap<String, RequiresPackagesInfo>, isPathOption: Bool,
        libName: String, tomlPath: String): Bool {
        let (flag, cjoInfo) = this.scanCjoInfo(cjoPath, this.globalConfig.isTest)
        if (!flag) {
            return false
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
                this.superPkgSubMap.add(pkgInfo.libName, pkgInfo.libName.split(".")[0])
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
    func scanCjoInfo(cjoPath: String, isTest: Bool): (Bool, RequiresPackages) {
        let timeStamp = FileInfo(cjoPath).lastModificationTime.toUnixTimeStamp().toSeconds()
        if (this.binDepCacheMap.contains(cjoPath) && (timeStamp == this.binDepCacheMap[cjoPath].timeStamp)) {
            return (true, this.binDepCacheMap[cjoPath])
        }
        let arguments = ArrayList<String>(["--scan-dependency", cjoPath])
        if (isTest) {
            arguments.add("--test")
        }

        var (scanFlag, outInfo, errInfo) = execWithOutput(COMPILE_TOOL, arguments)
        if (!scanFlag) {
            eprintln(errInfo)
            eprintln("Error: failed to obtain package import information of '${cjoPath}'")
            return (false, RequiresPackages())
        }

        var p = RequiresPackages()
        try {
            p = RequiresPackages.deserialize(DataModel.fromJson(JsonValue.fromStr(outInfo)))
        } catch (e: Exception) {
            eprintln(e.toString())
            eprintln("Error: failed to parse package import information of '${cjoPath}'")
            println(outInfo)
            return (false, RequiresPackages())
        }

        // CANGJIE_PATH check.
        let rootPkgName: String = getRootPkgName(p.packageName)
        let (_, rootPath) = makePackagePath(cjoPath)
        if (!Path(rootPath).normalize().toString().endsWith(Path(rootPkgName).normalize().toString())) {
            eprintln("Error: the file '${cjoPath}' must be placed in the '${rootPkgName}' folder")
            return (false, RequiresPackages())
        }

        p.timeStamp = timeStamp

        synchronized(this.scanCjoMtx) {
            this.binDepCacheMap.add(cjoPath, p)
        }

        return (true, p)
    }

    // read incremental cache of bin-dependencies
    func loadBinIncrementalCache(): Unit {
        let binCachePath = Path(this.globalConfig.targetDir).join(BIN_INCREMENTAL_CACHE).toString()
        if (!fileExists(binCachePath)) {
            return
        }
        try {
            var buf = String.fromUtf8(File.readFrom(binCachePath))
            this.binDepCacheMap = HashMap<String, RequiresPackages>.deserialize(
                DataModel.fromJson(JsonValue.fromStr(buf)))
        } catch (e: Exception) {
            eprintln("Warning: load ${binCachePath} failed\n${e.toString()}")
            return
        }
    }

    // update incremental cache of bin-dependencies
    func updateBinIncrementalCache(): Unit {
        let binCachePath = Path(this.globalConfig.targetDir).join(BIN_INCREMENTAL_CACHE).toString()
        let cacheString = this.binDepCacheMap.serialize().toJson().toJsonString()
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

            if (!(v.outputType.isEmpty() || OUTPUT_TYPE_LIST.contains(v.outputType))) {
                eprintln("Error: the output-type '${v.outputType}' is invalid which is listed " +
                    "in 'package-configuration' field at ${getPath(pkg.path, CONFIG_FILE_NAME)}")
                checkFlag = false
            }
            if (OUTPUT_TYPE_LIST.contains(v.outputType)) {
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
    func processDependencyModule(requires: HashMap<String, DepInfo>, prePath: String, isLocked: Bool): BuildCheckResult {
        var buildCheckFlag = true
        var mockUsed = false
        for ((name, v) in requires) {
            // convert to absolute path for future comparisons
            if (let Some(vpath) <- v.path) {
                v.path = getPath(prePath, vpath)
            }

            if (isLocked) {
                v.importedPath = "cjpm.lock"
            } else {
                v.importedPath = Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()
            }

            var finalDepInfo: DepInfo = v
            if (let Some(current) <- this.depMap.get(name)) {
                if (this.replaceMap.contains(name)) {
                    continue
                }
                let (isAlternative, errInfo) = current.isAlternative(v)
                let isGitLocalDep = checkGitLocalDep(name, current, v, this.depPathMap)
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
            }

            this.depMap.add(name, finalDepInfo)

            let depInfo: DepInfo = this.depMap[name]
            let (flag, isOnlineDep, depPath) = getDepPath(name, depInfo, this.globalConfig)
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
    func processAllPkgs(tomlInfo: TomlInfo): BuildCheckResult {
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
                    pkgInfo.packagePath = pkg.path
                    pkgInfo.hasProdFiles = pkg.hasProdFiles
                    pkgInfo.hasTestFiles = pkg.hasTestFiles
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

    func runPackageTask(pkgInfo: ResolveItem, fileNameHash: String, timeStamp: Int64, directDeps: HashSet<String>): BuildCheckResult {
        let (flag, info) = getPackageInfo(pkgInfo.packagePath, pkgInfo.compileOption, pkgInfo.customizedOption)
        if (!flag) {
            return FAILED_BUILD_CHECK_RESULT
        }
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
        if (pkgInfo.modulePath == this.globalConfig.modulePath && !pkgInfo.packagePath.endsWith(pkgInfo.srcDir)) {
            if (pkgInfo.outputType == EXE_TYPE) {
                pkgInfo.outputType = STATIC_TYPE
            }
        }

        if (pkgInfo.isMacroPackage) {
            // The output type of macro package should be 'dynamic'.
            if (pkgInfo.outputType == STATIC_TYPE) {
                pkgInfo.outputType = DYNAMIC_TYPE
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
        if (!this.depTomlMap.contains(rootPkgName)) {
            // bin-dependencies
            return false
        }
        let tomlInfo: TomlInfo = this.depTomlMap[rootPkgName]
        var flag: Bool = false
        if (dep == rootPkgName) {
            flag = (tomlInfo.pkg.outputType == EXE_TYPE)
        }
        if (tomlInfo.pkg.packageConfiguration.contains(dep)) {
            flag = (tomlInfo.pkg.packageConfiguration[dep].outputType == EXE_TYPE)
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
        let pkgPath: String = pkgInfo.packagePath.removePrefix(pkgInfo.srcDir)
        let actualName: String = getActualName(pkgPath, pkgInfo.rootPkgName)
        if (pkgInfo.fullName != actualName) {
            synchronized(this.errPrintMtx) {
                eprintln("Error: the package name in ${pkgInfo.packagePath} is wrong, " +
                    "the right name should be '${actualName}'")
            }
            return false
        }
        return true
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

    func getPackageInfo(pkgPath: String, cmdOption: String, customizedMap: HashMap<String, String>): (Bool, RequiresPackages) {
        if (!sourcePathCheck(pkgPath)) {
            synchronized(this.errPrintMtx) {
                eprintln("Error: the '${pkgPath}' contains illegal characters")
            }
            return (false, RequiresPackages())
        }

        var arguments = ArrayList<String>(["-p", pkgPath, "--scan-dependency"])
        if (this.globalConfig.isTest) {
            arguments.add("--test")
        }
        var cfgOption = ""
        if (!cmdOption.isEmpty() && cmdOption.contains("--cfg")) {
            cfgOption += cmdOption
        }
        for (customizedKey in this.globalConfig.customizedOption) {
            if (customizedMap.contains(customizedKey) && customizedMap[customizedKey].contains("--cfg")) {
                cfgOption += " ${customizedMap[customizedKey]}"
            }
        }
        arguments.add(all: extractCfgOption(cfgOption))

        let (scanFlag, outInfo, errInfo) = execWithOutput(COMPILE_TOOL, arguments)
        let cmd: String = COMPILE_TOOL + " " + String.join(arguments.toArray(), delimiter: " ")
        if (!scanFlag) {
            synchronized(this.errPrintMtx) {
                eprintln(errInfo)
                eprintln("Error: failed to obtain package import information, please execute '${cmd}' to check")
            }
            return (false, RequiresPackages())
        }

        var res = RequiresPackages()
        try {
            res = RequiresPackages.deserialize(DataModel.fromJson(JsonValue.fromStr(outInfo)))
        } catch (e: Exception) {
            synchronized(this.errPrintMtx) {
                eprintln("Error: failed to parse package import information, please execute '${cmd}' to check")
                println(outInfo)
            }
            return (false, RequiresPackages())
        }

        return (true, res)
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
        jsonData.add(tomlInfo.pkg.outputType)
        jsonData.add(tomlInfo.pkg.compileOption)
        jsonData.add(tomlInfo.pkg.linkOption)
        for ((k, v) in tomlInfo.profile.customizedOption) {
            jsonData.add(k)
            jsonData.add(v)
        }
        for ((k, v) in tomlInfo.pkg.packageConfiguration) {
            jsonData.add(k)
            jsonData.add(v.outputType)
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
            for (subPkg in superPkgCfg.subPkgSet) {
                let extraSuperPkgDep: HashSet<String> = HashSet<String>()
                getDepBetweenSuperPkg(subPkg, extraSuperPkgDep, checkedPkgSet)
                this.packageDepMap[superPkgName].requires.add(all: extraSuperPkgDep)
            }
            this.packageDepMap[superPkgName].requires.remove(superPkgName)

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
        for (dep in this.packageDepMap[pkgName].requires) {
            let superPkgName = dep.split(".")[0]
            if (this.superPkgMap.contains(superPkgName)) {
                extraSuperPkgDep.add(superPkgName)
            }
            getDepBetweenSuperPkg(dep, extraSuperPkgDep, checkedPkgSet)
        }
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

// get all package information for current module.
func getAllPkgs(srcPath: String, rootPkgName: String): Option<ArrayList<PackageInfo>> {
    var packageSrcPath = ArrayList<String>([srcPath])
    // Get valid package directory.
    var validPkgs = ArrayList<PackageInfo>()
    var pathDepth = 0
    // Get all directory.
    do {
        pathDepth = pathDepth + 1
        var temp = ArrayList<String>()
        for (pkgDirStr in packageSrcPath) {
            if (!directoryExists(pkgDirStr)) {
                eprintln("Error: directory '${pkgDirStr}' does not exist or path is too long")
                return Option.None
            }
            if (!collectSinglePkg(validPkgs, pkgDirStr)) {
                eprintln("Warning: there is no '.cj' file in directory '${pkgDirStr}', " +
                    "and its subdirectories will not be scanned as source code")
                continue
            }
            for (dirInfo in getDirectoryList(pkgDirStr)) {
                temp.add(dirInfo.path.toString())
            }
        }
        packageSrcPath = temp
    } while (packageSrcPath.size != 0)

    if (pathDepth > 20) {
        eprintln("Warning: the module depth of the '${rootPkgName}' is more than 20")
    }

    return validPkgs
}

func collectSinglePkg(validPkgs: ArrayList<PackageInfo>, pkgDir: String): Bool {
    var hasTestFile = false
    var hasProdFile = false
    var fileList = ArrayList<String>()
    var timeStamp: Int64 = Int64.Min
    for (fileInfo in getFileList(pkgDir)) {
        let fileName = fileInfo.path.toString()
        if (fileName.endsWith("_test.cj")) {
            hasTestFile = true
            fileList.add(fileName)
            timeStamp = max(timeStamp, fileInfo.lastModificationTime.toUnixTimeStamp().toSeconds())
        } else if (fileName.endsWith(".cj")) {
            hasProdFile = true
            fileList.add(fileName)
            timeStamp = max(timeStamp, fileInfo.lastModificationTime.toUnixTimeStamp().toSeconds())
        }
    }
    if (hasTestFile || hasProdFile) {
        sort(fileList, by: cmpStr, stable: true)
        validPkgs.add(PackageInfo(canonicalize(pkgDir).toString(),
            bufferHashCode(fileList), timeStamp, hasTestFile, hasProdFile))
        return true
    }
    return false
}

public func checkGitLocalDep(name: String, curInfo: DepInfo, newInfo: DepInfo,
    depPathMap: HashMap<String, String>): Bool {
    if (!(curInfo.isGitDep() && newInfo.isPathDep()) || !depPathMap.contains(name)) {
        return false
    }
    return depPathMap[name] == newInfo.path.getOrDefault({=> ""})
}
