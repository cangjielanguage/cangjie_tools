// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.time.*
import std.convert.*
import std.env.*
import std.fs.*
import stdx.encoding.json.*
import stdx.serialization.serialization.*
import cjpm.config.*
import cjpm.util.*

public let FIXED_SUBPACKAGE = HashSet<String>()

// Command build implement.
public func doBuild(tomlInfo: TomlInfo, buildConfig: BuildConfig, isScriptDep!: Bool = false): Bool {
    let curResolveData: ModuleResolve = generateResolveData(buildConfig, tomlInfo, isScriptDep) ?? return false

    // record '.cjpm-history' file at originTargetPath
    let resolveString = curResolveData.serialize().toJson().toJsonString()
    let historyFile = Path(buildConfig.originTargetPath).join(HISTORY_FILE_NAME).toString()
    if (!createAndWriteFile(historyFile, resolveString)) {
        return false
    }

    (buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir) = setFinalTargetDirectory(
        buildConfig.globalConfig, buildConfig.isCrossCompile, buildConfig.isDebug, buildConfig.mockSupported)
    prepareBuild(buildConfig, curResolveData)
    appendNativeData(buildConfig, curResolveData)

    if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
        buildConfig.globalConfig.compilePerformanceTargetDir = Path(buildConfig.globalConfig.targetDir).join(COMPILE_PERFORMANCE_DIR).toString()
        deleteDirectory(buildConfig.globalConfig.compilePerformanceTargetDir)
        if (!createDirectory(buildConfig.globalConfig.compilePerformanceTargetDir)) {
            return false
        }
    }

    let (ldPath, cLibLinkMap, crossCLibLinkMap, cjPathList) = configureModuleResolveData(
        buildConfig.globalConfig.targetDir, buildConfig.globalConfig.nativeDir, curResolveData,
        macroDep: buildConfig.pkgRequireByMacro)
    buildConfig.globalConfig.cLibLinkMap = cLibLinkMap.clone()
    buildConfig.globalConfig.crossCLibLinkMap = crossCLibLinkMap.clone()
    buildConfig.globalConfig.ldPath.add(all: ldPath)
    buildConfig.globalConfig.cjPathList.add(all: cjPathList)

    // Create the bin directory.
    let binDirectory = Path(buildConfig.globalConfig.targetDir).join(BIN).toString()
    if (!createDirectory(binDirectory)) {
        return false
    }

    if (!hasRealOption(buildConfig.customizedOption, curResolveData.resolves)) {
        return false
    }

    if (buildConfig.isIncremental) {
        analyseIncrementalCache(buildConfig, curResolveData)
        collectRebuildMacros(buildConfig)
    }

    if (!startBuild(buildConfig, curResolveData)) {
        return false
    }

    if (!buildConfig.globalConfig.isTest && buildConfig.globalConfig.isVerbose && buildConfig.packageList.exe.size > 0) {
        println("   Finished `build` profile compilation")
        println("   Executable files at `${Path(buildConfig.globalConfig.targetDir).join(BIN)}`")
    }

    if (buildConfig.globalConfig.isSkipScript) {
        return true
    }

    if (!runMultiScriptPost(buildConfig.globalConfig)) {
        return false
    }

    if (!isScriptDep) {
        if (!runScript(buildConfig.scriptConfig, Post, "build")) {
            return false
        }

        if (buildConfig.globalConfig.isInstall) {
            if (!runScript(buildConfig.scriptConfig, Post, "install")) {
                return false
            }
        }
    }

    return true
}

func recordCjpmBuildStartTime(tomlInfo: TomlInfo) {
    if (tomlInfo.profile.build.analysisCompilePerformance) {
        let cjpmCommand: String = String.join(getCommandLine(), delimiter:" ")
        COMMAND_CACHE.add(CommandInfo("cjpm", cjpmCommand))
        SHOW_CACHE.add(PkgInfo("cjpm", "B", DateTime.now().toUnixTimeStamp().toMilliseconds()))
        memProcess = addMemoryProcess()
    }
}

func recordCjpmBuildEndTime(tomlInfo: TomlInfo, path: String) {
    if (tomlInfo.profile.build.analysisCompilePerformance) {
        memProcess.cancel()
        memProcess.get()
        var cjpmEndTime = DateTime.now().toUnixTimeStamp().toMilliseconds()
        SHOW_CACHE.add(PkgInfo("cjpm", "E", cjpmEndTime))
        CJPM_COMPILE_INFO.add(cjpmEndTime.toString(),getCjpmMemInfo())
        addCompileCost(path)
    }
}

// Command build implement.
public func doBuild(buildConfig: BuildConfig): Bool {
    let tomlInfo: TomlInfo = loadModuleFile(buildConfig.globalConfig.rootPath) ?? return false
    recordCjpmBuildStartTime(tomlInfo)
    let buildRes = doBuild(tomlInfo, buildConfig, isScriptDep: false)
    recordCjpmBuildEndTime(tomlInfo, buildConfig.globalConfig.compilePerformanceTargetDir)
    return buildRes
}

public func getGlobalFolderPath(): Path {
    let path = GLOBAL_CJPM_CONFIG_DIR

    if (!directoryExists(path.toString())) {
        try {
            Directory.create(path)
        } catch (e: Exception) {
            eprintln("Error: can't create global cjpm config folder(${path}).")
        }
    }
    canonicalize(path)
}

func hasRealOption(cmdOption: HashSet<String>, resolves: ArrayList<ResolveItem>): Bool {
    if (cmdOption.size == 0) {
        return true
    }

    let optionSet = HashSet<String>()
    for (res in resolves) {
        optionSet.add(all: res.customizedOption.keys())
    }

    for (v in cmdOption) {
        if (!optionSet.contains(v)) {
            eprintln("Error: can not find the customized key '${v}' in all cjpm.toml")
            return false
        }
    }
    return true
}

// Resolve dependencies and generate .cjpm-history file.
func generateResolveData(buildConfig: BuildConfig, tomlInfo: TomlInfo, isScriptDep: Bool): Option<ModuleResolve> {
    if (!replaceEnvforTargetDir(tomlInfo, Path(buildConfig.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString())) {
        return Option<ModuleResolve>.None
    }
    var configFileDir: String
    if (tomlInfo.isPkgConfig) {
        configFileDir = tomlInfo.pkg.targetDir
        buildConfig.curModuleName = tomlInfo.pkg.name
    } else {
        configFileDir = tomlInfo.workspace.targetDir
    }

    buildConfig.globalConfig.targetDir = getOriginTargetDirectory(buildConfig.globalConfig.targetDir,
        buildConfig.globalConfig.rootPath, buildConfig.targetDir, configFileDir, true) ??
        return Option<ModuleResolve>.None
    buildConfig.globalConfig.originDir = buildConfig.globalConfig.targetDir
    buildConfig.originTargetPath = buildConfig.globalConfig.targetDir

    if (!replaceEnvforScriptDir(tomlInfo, Path(buildConfig.globalConfig.rootPath))) {
            return Option<ModuleResolve>.None
    }

    if (!buildConfig.globalConfig.isSkipScript && !isScriptDep) {
        let configScriptFileDir = getScriptDirValue(tomlInfo)
        buildConfig.globalConfig.scriptDir = getScriptDirectory(buildConfig.globalConfig.scriptDir,
            buildConfig.globalConfig.rootPath, configScriptFileDir, true) ??
            return Option<ModuleResolve>.None
        let scriptConfig: ScriptConfig = makeScriptConfigByToml(buildConfig.globalConfig.scriptDir,
            buildConfig.globalConfig.rootPath, tomlInfo, buildConfig.isDebug, buildConfig.isVerbose,
            buildConfig.isCrossCompile) ?? return Option.None
        if (buildConfig.globalConfig.isInstall) {
            if (!runScript(scriptConfig, Pre, "install")) {
                return Option<ModuleResolve>.None
            }
        }
        if (!runScript(scriptConfig, Pre, "build")) {
            return Option<ModuleResolve>.None
        }
        buildConfig.scriptConfig = scriptConfig
    }

    if (!checkForWorkspace(buildConfig.globalConfig.rootPath, tomlInfo, buildConfig.memberModule,
        isDebug: buildConfig.isDebug, isTest: buildConfig.globalConfig.isTest, checkWorkspaceDir: !isScriptDep)) {
        return Option<ModuleResolve>.None
    }
    if (buildConfig.isInstall && !COMMON_INFO.inWorkspace && tomlInfo.pkg.outputType != Exe) {
        eprintln("Error: output-type in " +
            "${Path(buildConfig.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString()} " +
            "must be '${Exe}' for cjpm install")
        return Option<ModuleResolve>.None
    }

    return getResolveData(buildConfig, tomlInfo, isScriptDep)
}

func getResolveData(buildConfig: BuildConfig, tomlInfo: TomlInfo, isScriptDep: Bool): Option<ModuleResolve> {
    buildConfig.globalConfig.customizedOption = buildConfig.customizedOption
    buildConfig.globalConfig.isForScript = isScriptDep
    let (dm, flag, _) = resolve(buildConfig.globalConfig, isScriptDep: isScriptDep)
    if (!flag) {
        return Option<ModuleResolve>.None
    }
    buildConfig.jsonInformation = dm.jsonMap
    buildConfig.pkgCacheMap = dm.pkgCacheMap
    buildConfig.globalConfig.scriptList = dm.globalConfig.scriptList
    buildConfig.globalConfig.overrideCompileOption = dm.globalConfig.overrideCompileOption
    buildConfig.globalConfig.nativeOverrideOption = dm.globalConfig.nativeOverrideOption

    if (buildConfig.globalConfig.isProjectCombine &&
        buildConfig.packageList.macros.contains(buildConfig.globalConfig.rootName)) {
        eprintln("Error: field " +
            "'package.package-configuration.${buildConfig.globalConfig.rootName}.combine-all-deps = true' " +
            "is not support for macro root package '${buildConfig.globalConfig.rootName}'")
        return Option<ModuleResolve>.None
    }

    if (!tomlInfo.profile.build.ltoValue.isEmpty()) {
        buildConfig.isLto = supportLto()
        buildConfig.ltoValue = tomlInfo.profile.build.ltoValue
    }

    if (supportLto() && buildConfig.globalConfig.isProjectCombine && buildConfig.ltoValue.isEmpty()) {
        eprintln("Error: field " +
            "'package.package-configuration.${buildConfig.globalConfig.rootName}.combine-all-deps = true' " +
            "is invalid when field 'profile.build.lto' is empty")
        return Option<ModuleResolve>.None
    } else if (!supportLto() && buildConfig.globalConfig.isProjectCombine) {
        eprintln("Error: field " +
            "'package.package-configuration.${buildConfig.globalConfig.rootName}.combine-all-deps = true' " +
            "only supports compilation to Linux/OHOS")
        return Option<ModuleResolve>.None
    }

    if (tomlInfo.profile.build.analysisCompilePerformance) {
        buildConfig.globalConfig.isAnalysisCompilePerformance = true
    }
    if (tomlInfo.profile.build.incremental) {
        buildConfig.isIncremental = true
    }

    let resolves = ArrayList<ResolveItem>()
    let superPkgList = ArrayList<SuperPackageConfig>()
    for (k in dm.readyPkgs) {
        var pkg = dm.packageDepMap[k]
        if (let Some(v) <- pkg.superPkgCfg) {
            superPkgList.add(v)
        }
        if (buildConfig.globalConfig.isProjectCombine) {
            if (pkg.requires.contains(buildConfig.globalConfig.rootName)) {
                eprintln("Error: roo package '${buildConfig.globalConfig.rootName}' is depended by package ${k}, " +
                    "it cannot be depended by any package when field 'combine-all-deps = true' is valid")
                return Option<ModuleResolve>.None
            }
            if (k != buildConfig.globalConfig.rootName && !pkg.isMacroPackage) {
                pkg.outputType = Static
            } else {
                pkg.outputType = Dynamic
            }
        }
        resolves.add(dm.packageDepMap[k])
    }
    buildConfig.binDepMap = dm.binDepMap
    buildConfig.crossBinDepMap = dm.crossBinDepMap
    buildConfig.targetMacroMods = dm.targetMacroMods
    buildConfig.targetMacroPkgs = dm.targetMacroPkgs
    buildConfig.superPkgMap = dm.superPkgMap
    buildConfig.superPkgSubMap = dm.superPkgSubMap
    return ModuleResolve(
        resolves: resolves,
        binDeps: dm.binDepMap,
        crossBinDeps: dm.crossBinDepMap
    )
}

func setFinalTargetDirectory(globalConfig: GlobalConfig, isCrossCompile: Bool, isDebug: Bool, isMock: Bool): (String,
    String) {
    var targetDir = globalConfig.targetDir
    var nativeDir = targetDir
    if (isCrossCompile) {
        targetDir = Path(targetDir).join(crossCompileTarget).toString()
    }

    if (isMock) {
        targetDir = Path(targetDir).join(MOCK).toString()
        nativeDir = Path(nativeDir).join(MOCK).toString()
    }

    if (isDebug) {
        nativeDir = Path(nativeDir).join(DEBUG).toString()
        targetDir = Path(targetDir).join(DEBUG).toString()
    } else {
        nativeDir = Path(nativeDir).join(RELEASE).toString()
        targetDir = Path(targetDir).join(RELEASE).toString()
    }

    // Set CANGJIE_PATH variable.
    globalConfig.cjPathList.add(targetDir)
    if (nativeDir != targetDir) {
        globalConfig.cjPathList.add(nativeDir)
    }

    return (targetDir, nativeDir)
}

// Determine whether to recompile.
func analyseIncrementalCache(buildConfig: BuildConfig, curResolveData: ModuleResolve): Unit {
    let lastFileNameHashMap = HashMap<String, String>()
    let lastTimeStampMap = HashMap<String, Int64>()
    let lastBuildFlagMap = HashMap<String, Bool>()
    let lastHasSubpkgsMap = HashMap<String, Bool>()
    for ((rootPkgName, jsonData) in buildConfig.jsonInformation) {
        let cacheFile = Path(buildConfig.globalConfig.targetDir).join(rootPkgName).join(INCREMENTAL_CACHE).toString()
        if (!fileExists(cacheFile)) {
            buildConfig.changedModuleList.add(rootPkgName)
            continue
        }
        let lastCache = loadCacheFile(cacheFile) ?? IncrementalCache()
        if (lastCache.jsonHash != bufferHashCode(jsonData)) {
            buildConfig.changedModuleList.add(rootPkgName)
            continue
        }
        for ((pkgName, v) in lastCache.packageInformation) {
            lastBuildFlagMap[pkgName] = v.isBuilt
            lastFileNameHashMap[pkgName] = v.fileNameHash
            lastTimeStampMap[pkgName] = v.timeStamp
            lastHasSubpkgsMap[pkgName] = v.hasSubPkgs
            if (!COMMON_INFO.inWorkspace && buildConfig.packageList.exe.size == 1 &&
                buildConfig.packageList.exe[0] == pkgName && v.exeName != buildConfig.exeName) {
                buildConfig.rebuildList.add(pkgName)
            }
        }
    }

    for (r in curResolveData.resolves) {
        if (buildConfig.changedModuleList.contains(r.rootPkgName) || !buildConfig.pkgCacheMap.contains(r.fullName)) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }
        if (!lastFileNameHashMap.contains(r.fullName) ||
            lastFileNameHashMap[r.fullName] != buildConfig.pkgCacheMap[r.fullName].fileNameHash) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }
        if (!lastBuildFlagMap.contains(r.fullName) || !lastBuildFlagMap[r.fullName]) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }

        let hasSubPkgs = buildConfig.hasSubPkgs.contains(r.fullName) || buildConfig.requiredForTests // in the current compilation
        if (!lastHasSubpkgsMap.contains(r.fullName) || (lastHasSubpkgsMap[r.fullName] != hasSubPkgs)) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }

        if (!lastTimeStampMap.contains(r.fullName) ||
            buildConfig.pkgCacheMap[r.fullName].timeStamp > lastTimeStampMap[r.fullName]) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }
        if (r.outputType == Exe && !buildConfig.requiredForTests) {
            if (!hasExeFile(buildConfig, r.fullName)) {
                buildConfig.rebuildList.add(r.fullName)
                continue
            }
        }
    }

    refreshIncreInfo(buildConfig, curResolveData)
}

func collectRebuildMacros(buildConfig: BuildConfig): Unit {
    for (name in buildConfig.rebuildList) {
        if (buildConfig.packageList.macros.contains(name)) {
            buildConfig.rebuildMacros.add(name)
        }
    }
}

func hasExeFile(buildConfig: BuildConfig, fullName: String): Bool {
    var exeName: String = buildConfig.exeName
    if (buildConfig.packageList.exe.size > 1 || COMMON_INFO.inWorkspace) {
        exeName = fullName
    }

    if (buildConfig.isCrossCompile) {
        exeName = makeTargetExeName(exeName, crossCompileTarget)
    } else {
        exeName = makeExeName(exeName)
    }

    let exeFile = Path(buildConfig.globalConfig.targetDir).join(BIN).join(exeName).toString()
    return fileExists(exeFile)
}

func refreshIncreInfo(buildConfig: BuildConfig, curResolveData: ModuleResolve): Unit {
    collectCjoTime(buildConfig)
    collectLastTime(buildConfig)
    var incrementalFlag: Bool = buildConfig.isIncremental
    while (incrementalFlag) {
        incrementalFlag = analyseRebuild(buildConfig)
    }

    for (r in curResolveData.resolves) {
        if (!buildConfig.rebuildList.contains(r.fullName)) {
            r.buildFlag = true
        }
        if (r.fullName == buildConfig.globalConfig.rootName && buildConfig.globalConfig.isProjectCombine &&
            !buildConfig.rebuildList.isEmpty()) {
            buildConfig.rebuildList.add(r.fullName)
            r.buildFlag = false
        }
    }
    return
}

// Record package information in buildConfig.
func prepareBuild(buildConfig: BuildConfig, curResolveData: ModuleResolve): Unit {
    for (r in curResolveData.resolves) {
        buildConfig.requiresMap[r.fullName] = r.requires
    }

    buildConfig.packageList = PackageList(curResolveData.resolves)

    for ((_, deps) in curResolveData.binDeps) {
        for ((fullName, info) in deps.packageRequires) {
            if (!buildConfig.requiresMap.contains(fullName)) {
                buildConfig.requiresMap[fullName] = info.requires
            }
        }
    }

    for (fullName in buildConfig.packageList.macros) {
        buildConfig.pkgRequireByMacro.add(all: getDepSet(fullName, buildConfig.requiresMap,
            buildConfig.superPkgSubMap, isForMacroRequire: true))
    }

    for ((_, deps) in curResolveData.crossBinDeps) {
        for ((fullName, info) in deps.packageRequires) {
            if (!buildConfig.requiresMap.contains(fullName)) {
                buildConfig.requiresMap[fullName] = info.requires
            }
        }
    }

    for ((_, jsonData) in buildConfig.jsonInformation) {
        jsonData.add(all: buildConfig.getOptionCacheData())
        if (buildConfig.isLto) {
            jsonData.add(buildConfig.ltoValue)
        }
    }

    for (pkgName in buildConfig.packageList.all) {
        if (buildConfig.packageList.exe.contains(pkgName)) {
            continue
        }
        let parentPkg: String = getParentPkgName(pkgName)
        if (buildConfig.packageList.all.contains(parentPkg) || buildConfig.requiredForTests) {
            buildConfig.hasSubPkgs.add(parentPkg)
        }
    }

    return
}

func collectCjoTime(buildConfig: BuildConfig): Unit {
    for (pkgName in buildConfig.packageList.all) {
        if (buildConfig.packageList.test.contains(pkgName)) {
            continue
        }
        if (buildConfig.packageList.macros.contains(pkgName) && !crossCompileTarget.isEmpty()) {
            collectMacroCjoTime(buildConfig, pkgName)
            continue
        }
        var cjoPath = ""
        let rootPkgName: String = getRootPkgName(pkgName)
        if (!buildConfig.packageList.exe.contains(pkgName) || buildConfig.requiredForTests) {
            cjoPath = Path(buildConfig.globalConfig.targetDir).join(rootPkgName).toString()
        } else {
            cjoPath = Path(buildConfig.globalConfig.targetDir).join(BIN).toString()
        }
        let cjoFile = Path(cjoPath).join("${pkgName}.cjo").toString()
        if (!fileExists(cjoFile)) {
            buildConfig.rebuildList.add(pkgName)
        } else {
            buildConfig.cjoTimeMap[pkgName] = FileInfo(cjoFile).lastModificationTime.toUnixTimeStamp().toSeconds()
        }
    }
    return
}

func collectLastTime(buildConfig: BuildConfig): Unit {
    let endTimeCacheFile = Path(buildConfig.globalConfig.targetDir)
        .join("${buildConfig.curModuleName}-cache.json")
        .toString()
    if (fileExists(endTimeCacheFile)) {
        let (time, flag) = loadStringFromFile(endTimeCacheFile)
        if (!flag) {
            return
        }
        try {
            buildConfig.lastEndTime = Int64.parse(time)
        } catch (_: Exception) {}
    }
    return
}

// Analyzing indirectly affected packages in incremental.
func analyseRebuild(buildConfig: BuildConfig): Bool {
    for ((pkgName, requires) in buildConfig.requiresMap) {
        if (buildConfig.rebuildList.contains(pkgName)) {
            continue
        }
        for (r in requires) {
            if (buildConfig.rebuildList.contains(r)) {
                buildConfig.rebuildList.add(pkgName)
                buildConfig.indirectRebuilds.add(pkgName)
                return true
            }
            if (buildConfig.packageList.test.contains(r)) {
                continue
            }
            if (buildConfig.cjoTimeMap.contains(r) && buildConfig.lastEndTime < buildConfig.cjoTimeMap[r]) {
                buildConfig.rebuildList.add(pkgName)
                return true
            }
        }
    }

    for ((superPkgName, superPkgCfg) in buildConfig.superPkgMap) {
        if (buildConfig.rebuildList.contains(superPkgName)) {
            continue
        }

        for (subPkg in superPkgCfg.subPkgSet) {
            if (buildConfig.rebuildList.contains(subPkg)) {
                buildConfig.rebuildList.add(superPkgName)
                return true
            }
        }
    }

    return false
}

// Call cjc to compile.
func startBuild(buildConfig: BuildConfig, res: ModuleResolve): Bool {
    var flag: Bool = true
    try {
        flag = parallelBuild(res, buildConfig)
    } catch (e: Exception) {
        return false
    }

    if (!generateIncrementalCache(buildConfig, res)) {
        return false
    }

    if (!flag) {
        return false
    }

    return buildCodeCheck(buildConfig)
}

func addCompileCost(compilePerformanceTargetDir : String): Unit {
    sortCompileCost(compilePerformanceTargetDir)
    var SHOW_CACHEPath = Path(compilePerformanceTargetDir).join("time_cost.json").toString()
    var COMMAND_CACHEPath = Path(compilePerformanceTargetDir).join("package_command.json").toString()
    var CJPM_COMPILE_INFOPath = Path(compilePerformanceTargetDir).join("memory_cost.json").toString()
    createAndWriteFile(SHOW_CACHEPath, SHOW_CACHE.serialize().toJson().toJsonString())
    createAndWriteFile(COMMAND_CACHEPath, COMMAND_CACHE.serialize().toJson().toJsonString())
    createAndWriteFile(CJPM_COMPILE_INFOPath,CJPM_COMPILE_INFO.serialize().toJson().toJsonString())
}

func sortCompileCost(compilePerformancePath : String): Unit {
    var startMap = HashMap<String, Int64>()
    var endMap = HashMap<String, Int64>()
    var allPkgs = ArrayList<String>()
    for (k in SHOW_CACHE) {
        if (k.ph == "B") {
            allPkgs.add(k.name)
            startMap[k.name] = k.ts
        } else {
            endMap[k.name] = k.ts
        }
    }
    let pkgSize = allPkgs.size
    var threadTime = Array<Int64>(pkgSize, repeat: 0)
    var threadQueue = HashMap<String, Int64>()
    for (name in allPkgs) {
        for (i in 0..threadTime.size) {
            if (startMap[name] >= threadTime[i]) {
                threadQueue[name] = i
                threadTime[i] = endMap[name]
                break
            }
        }
    }

    for (cache in SHOW_CACHE) {
        cache.tid = threadQueue[cache.name]
    }

    var timeCache = HashMap<String, String>()
    for (k in allPkgs) {
        timeCache[k] = ((endMap[k] - startMap[k]) * Duration.millisecond).toString()
    }
    var timeCachePath = Path(compilePerformancePath).join("time-cache.json").toString()
    createAndWriteFile(timeCachePath, timeCache.serialize().toJson().toJsonString())

    return
}

func generateIncrementalCache(buildConfig: BuildConfig, curResolveData: ModuleResolve): Bool {
    var successPkgs = ArrayList<String>()
    for (item in curResolveData.resolves) {
        if (item.buildFlag) {
            successPkgs.add(item.fullName)
        }
    }

    var cacheMap = HashMap<String, IncrementalCache>()
    for ((rootPkgName, jsonData) in buildConfig.jsonInformation) {
        cacheMap[rootPkgName] = IncrementalCache()
        cacheMap[rootPkgName].jsonHash = bufferHashCode(jsonData)
        cacheMap[rootPkgName].rootPkgName = rootPkgName
    }

    for ((pkgName, pkgData) in buildConfig.pkgCacheMap) {
        if (!cacheMap.contains(pkgData.rootPkgName)) {
            continue
        }
        let cacheItem = CacheItem(pkgData.rootPkgName, pkgData.fileNameHash, pkgData.timeStamp,
            buildConfig.hasSubPkgs.contains(pkgName) || buildConfig.requiredForTests)
        if (successPkgs.contains(pkgName)) {
            cacheItem.isBuilt = true
        }

        cacheMap[pkgData.rootPkgName].packageInformation[pkgName] = cacheItem
    }

    if (buildConfig.packageList.exe.size == 1 && !COMMON_INFO.inWorkspace) {
        let fullName = buildConfig.packageList.exe[0]
        let rootPkgName = getRootPkgName(fullName)
        if (cacheMap.contains(rootPkgName) && cacheMap[rootPkgName].packageInformation.contains(fullName)) {
            cacheMap[rootPkgName].packageInformation[fullName].exeName = buildConfig.exeName
        }
    }

    var cacheFlag = true
    for ((k, v) in cacheMap) {
        let cachePath = Path(buildConfig.globalConfig.targetDir).join(swapOrgName(k))
        if (!createDirectory(cachePath.toString())) {
            cacheFlag = false
            continue
        }
        let cacheFile = cachePath.join(INCREMENTAL_CACHE).toString()
        let cacheStr = v.serialize().toJson().toJsonString()
        if (!createAndWriteFile(cacheFile, cacheStr)) {
            cacheFlag = false
        }
    }

    let endTimeCacheFile = Path(buildConfig.globalConfig.targetDir)
        .join("${buildConfig.curModuleName}-cache.json")
        .toString()
    let endTime: String = DateTime.now().toUnixTimeStamp().toSeconds().toString()
    if (!createAndWriteFile(endTimeCacheFile, endTime)) {
        cacheFlag = false
    }

    return cacheFlag
}

func constructCjcInvocation(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res: ArrayList<String> = ArrayList<String>()

    res.add(all: appendImportPaths(compileTask, buildConfig))
    res.add(all: appendCommandOptions(compileTask))
    res.add(all: getDepLinkCommand(compileTask, buildConfig))

    for (option in compileTask.customizedOption) {
        res.add(all: extractOptionByString(option))
    }

    res.add(all: extractOptionByString(compileTask.compileOption))
    res.add(all: extractOptionByString(compileTask.overrideOption))
    res.add(all: appendLinkOption(compileTask))
    res.add(all: getTypeCommand(compileTask))
    return res
}

func appendImportPaths(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res = ArrayList<String>()

    // When user wants to use multiplatform dependency without writing a multiplatform project
    // we must give user access to all desirable source set scopes
    let features = if (compileTask.isMultiplatform) {
        compileTask.featureMapping.addFeature(compileTask.sourceSetFeatures).collect()
    } else { compileTask.allEnabledFeatures }

    for (p in buildConfig.globalConfig.cjPathList) {
        if (directoryExists(p)) {
            res.add("--import-path=${p}")
        }
    }

    let dependencies = getDepSet(compileTask.fullName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
                isForMacroRequire: buildConfig.packageList.macros.contains(compileTask.fullName))
    let bestFitFeatures = buildConfig.packageList.bestFitFeatures(features)
    let CJMPPathSet = HashSet<String>()
    for (depName in dependencies) {
        if (let Some(suffix) <- bestFitFeatures.get(depName)) {
            let depRootName: String = getRootPkgName(depName)
            let libPath = Path(compileTask.targetDir).join(swapOrgName(depRootName)).join(suffix).toString()
            CJMPPathSet.add(libPath)
        }
    }
    res.add(all: CJMPPathSet |> map { p => "--import-path=${p}"} |> collectArray)
    return res
}

func appendCommandOptions(compileTask: CompileTask): ArrayList<String> {
    let res = ArrayList<String>()
    if (state.isExperimental) {
        res.add("--experimental")
    }
    if (compileTask.isCrossCompile) {
        res.add("--target=${crossCompileTarget}")
    }
    if (compileTask.isDebug) {
        res.add("-g")
    }
    if (compileTask.isCov) {
        res.add("--coverage")
    }
    if (compileTask.mockSupported) {
        res.add("--mock=on")
    }
    if (compileTask.outputType != Chir && !compileTask.hasSubPkgs && !compileTask.requiredForTests) {
        res.add("--no-sub-pkg")
    }
    res.add("--output-dir=${compileTask.targetPath}")
    res
}

func appendLinkOption(compileTask: CompileTask): ArrayList<String> {
    if (compileTask.linkOption.isEmpty()) {
        return ArrayList<String>()
    }
    if (compileTask.outputType == Static && !compileTask.requiredForTests) {
        return ArrayList<String>()
    }
    return ArrayList<String>(["--link-options", compileTask.linkOption])
}

func getDepLinkCommand(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res = ArrayList<String>()

    // module-level combine
    if (compileTask.isCombined) {
        res.add(all: getBuildArgs(compileTask, buildConfig))
        res.add(all: getForcedLibArgs(compileTask, buildConfig))
        return res
    }

    if (compileTask.outputType == Exe || compileTask.outputType == Dynamic) {
        if (compileTask.isProjectCombined) {
            res.add(all: getLtoCombineBuildArgs(compileTask, buildConfig))
        } else {
            res.add(all: getBuildArgs(compileTask, buildConfig))
        }
    }

    return res
}

func getTypeCommand(compileTask: CompileTask): ArrayList<String> {
    let res = ArrayList<String>()


    if (compileTask.isAnalysisCompilePerformance) {
        res.add("--profile-compile-time")
        res.add("--profile-compile-memory")
    }
    res.add(all: ["-p", compileTask.packagePath])
    if (compileTask.exportForTests) {
        res.add("--export-for-test")
    }

    if (let Some((chirpath, commonCjo)) <- compileTask.prevStageArtifacts) {
        res.add(chirpath)
        res.add("--common-part-cjo=${commonCjo}")
    }

    if (compileTask.isMacro) {
        assertion { compileTask.outputType == Exe || compileTask.outputType == Dynamic }
        // There's an edgecase, when package with `macro package` header 
        // Does reexport of other macro package but is being compiled as executable
        if (compileTask.outputType == Dynamic) {
            res.add("--compile-macro")
            return res
        } 
    }


    if (compileTask.isLto) {
        assertion { compileTask.outputType == Static || compileTask.outputType == Exe }
        res.add("--lto=${compileTask.ltoValue}")
    }

    res.add("--output-type=${compileTask.outputType.asCompilerArg}")

    match (compileTask.outputType) {
        case Static => 
            let name = if (compileTask.isLto) {
                makeLtoName(compileTask.filename)
            } else {
                makeCangjieStaticlibName(compileTask.filename)
            }
            res.add("-o=${name}")
        case Dynamic => res.add("-o=${makeTargetDylibName(compileTask.filename, compileTask.target)}")
        case Exe => res.add("-o=${makeTargetExeName(compileTask.filename, compileTask.target)}")
        case _ => ()
    }

    res.add(all: compileTask.supressed)
    return res
}

// add bin-dependencies links to build arguments
func appendOptionForBindep(binDepMap: HashMap<String, BinDeps>, linkPkgs: HashSet<String>, isLto: Bool): ArrayList<String> {
    let linkerOptions = ArrayList<String>()     // -l or lto libs
    let libPathOptions = ArrayList<String>()    // -L path
    let moduleLinkerSet = HashSet<String>()
    for ((_, eachModuleDepMap) in binDepMap) {
        let moduleLinkerOptions = ArrayList<String>()
        for (index in (eachModuleDepMap.packageRequiresSort.size - 1)..=0 : -1) {
            let depName = eachModuleDepMap.packageRequiresSort[index]
            if (!linkPkgs.contains(depName)) {
                continue
            }
            let info = eachModuleDepMap.packageRequires[depName]
            if (info.isMacroPackage) {
                continue
            }
            let linkArray: ArrayList<String> = linkPackage(info.libName, info.libPath, isLto)

            // size == 1 when isLto
            if (linkArray.size <= 1) {
                moduleLinkerOptions.add(all: linkArray)
                continue
            }

            // size == 3 when !isLto: -L path -llib
            if (!libPathOptions.contains(linkArray[1])) {
                libPathOptions.add(linkArray[0])
                libPathOptions.add(linkArray[1])
            }
            moduleLinkerOptions.add(linkArray[2])
        }

        // deal with module-level duplication
        let moduleLinkerStr = String.join(moduleLinkerOptions.toArray())
        if (!moduleLinkerSet.contains(moduleLinkerStr)) {
            moduleLinkerSet.add(moduleLinkerStr)
            linkerOptions.add(all: moduleLinkerOptions)
        }
    }

    libPathOptions.add(all: linkerOptions)
    return libPathOptions
}

func getCLibLinkStr(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>): ArrayList<String> {
    var res = ArrayList<String>()
    let cLibNameSet = HashSet<String>(cLibNameList)
    for (cLibName in cLibNameSet) {
        if (cLibLinkMap.contains(cLibName)) {
            res.add(all: cLibLinkMap[cLibName])
        }
    }
    return res
}

func getMinimizedCLibLinkStr(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>): ArrayList<String> {
    let cLibLinkStr = getCLibLinkStr(cLibLinkMap, cLibNameList)

    let res = ArrayList<String>()

    var index: Int64 = 0
    let cLibPathSet: HashSet<String> = HashSet<String>()
    let cLibNameSet: HashSet<String> = HashSet<String>()
    while (index < cLibLinkStr.size) {
        if (cLibLinkStr[index] == "-L") {
            if (!cLibPathSet.contains(cLibLinkStr[index + 1])) {
                res.add(cLibLinkStr[index])
                res.add(cLibLinkStr[index + 1])
                cLibPathSet.add(cLibLinkStr[index + 1])
            }
            index += 2
        } else if (cLibLinkStr[index].startsWith("-l")) {
            if (!cLibNameSet.contains(cLibLinkStr[index])) {
                res.add(cLibLinkStr[index])
                cLibNameSet.add(cLibLinkStr[index])
            }
            index += 1
        } else {
            res.add(cLibLinkStr[index])
            index += 1
        }
    }

    return res
}

func getLtoCombineBuildArgs(compileTask: CompileTask, buildConfig: BuildConfig):
    ArrayList<String> {
    let res = ArrayList<String>()
    var cLibNameList = ArrayList<String>([compileTask.rootPkgName]) // Record dependent root packages
    var linkSort: HashSet<String> = getDepSet(compileTask.fullName, buildConfig.requiresMap,
        buildConfig.superPkgSubMap,
        isForMacroRequire: buildConfig.packageList.macros.contains(compileTask.fullName)) // Record dependent package names
    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    var binDepSet = HashSet<String>()
    // link all lto packages
    // and get all binary dependencies depended by root package
    for (index in (buildConfig.packageList.all.size - 1)..=0 : -1) {
        let depName = buildConfig.packageList.all[index]
        if (linkSort.contains(depName)) {
            linkSort.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
            isForMacroRequire: buildConfig.packageList.macros.contains(depName)))
        }
        let depRootName: String = getRootPkgName(depName)
        cLibNameList.add(depRootName, at: 0)
        if (buildConfig.packageList.macros.contains(depName) || buildConfig.packageList.test.contains(depName)) {
            continue
        }

        var libPath = Path(compileTask.targetDir).join(swapOrgName(depRootName)).toString()

        let linkArray: ArrayList<String> = linkPackage(depName, libPath, compileTask.isLto)
        if (linkArray.size <= 1) {
            allOptions.add(all: linkArray)
            binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
                isForMacroRequire: buildConfig.packageList.macros.contains(depName)))
            continue
        }
        if (libPathSet.contains(libPath)) {
            allOptions.add(linkArray[linkArray.size - 1])
        } else {
            allOptions.add(all: linkArray)
            libPathSet.add(libPath)
        }
        binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
            isForMacroRequire: buildConfig.packageList.macros.contains(depName)))
    }
    res.add(all: allOptions)

    // link all binary dependecies depended by root package
    let binOptions = ArrayList<String>()
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, linkSort, compileTask.isLto), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, linkSort, compileTask.isLto), at: 0)
    res.add(all: binOptions)

    // link all binary dependencies depended by other packages which are not depended by root package
    let otherBinOptions = ArrayList<String>()
    binDepSet.remove(all: linkSort)
    otherBinOptions.add(all: getForcedBinDep(compileTask, buildConfig, binDepSet, cLibNameList))

    let compileTarget = compileTask.target
    addForcedLibArgs(res, otherBinOptions, compileTarget)

    // forced-add std dynamic libs
    if (let Some(stdLinks) <- addStdLibsLink(compileTarget)) {
        res.add(all: stdLinks)
    } else {
        println("Warning: cannot find std dynamic libraries for target '${compileTarget}' " +
            "from $CANGJIE_HOME/runtime/lib while compiling combine package ${compileTask.fullName}, " +
            "output may be invalid if sub packages import std libraries")
    }

    let cLibLinkMap = if (compileTask.isCrossCompile) {
        buildConfig.globalConfig.crossCLibLinkMap
    } else { buildConfig.globalConfig.cLibLinkMap }
    res.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))
    res.add("--lto=${buildConfig.ltoValue}")

    return res
}

func getBuildArgs(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res = ArrayList<String>()
    // Record dependent root packages
    let cLibLinkMap = if (compileTask.isCrossCompile) {
        buildConfig.globalConfig.crossCLibLinkMap
    } else { buildConfig.globalConfig.cLibLinkMap }
    let cLibNameList = ArrayList<String>([compileTask.rootPkgName])

    // Record dependent package names
    let superPkgMap: ?HashMap<String, SuperPackageConfig> = if (compileTask.isCombined) { buildConfig.superPkgMap } else { None }
    let linkSort: HashSet<String> = getDepSet(
        compileTask.fullName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
        isForMacroRequire: buildConfig.packageList.macros.contains(compileTask.fullName),
        superPkgMap: superPkgMap
    ) 

    if (linkSort.size == 0) {
        res.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))
        return res
    }

    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    for (index in (buildConfig.packageList.all.size - 1)..=0 : -1) {
        let depName = buildConfig.packageList.all[index]
        if (!linkSort.contains(depName)) {
            continue
        }

        let depRootName: String = getRootPkgName(depName)
        cLibNameList.add(depRootName, at: 0)
        if (buildConfig.packageList.macros.contains(depName) || buildConfig.packageList.test.contains(depName)) {
            continue
        }

        let libPath: String = if (let Some(suffix) <- buildConfig.packageList.productSuffixes.get(depName)) {
            Path(compileTask.targetDir).join(swapOrgName(depRootName)).join(suffix).toString()
        } else {
            Path(compileTask.targetDir).join(swapOrgName(depRootName)).toString()
        }

        let linkArray: ArrayList<String> = linkPackage(depName, libPath, compileTask.isLto)
        if (linkArray.size <= 1) {
            allOptions.add(all: linkArray)
            continue
        }
        if (libPathSet.contains(libPath)) {
            allOptions.add(linkArray[linkArray.size - 1])
        } else {
            allOptions.add(all: linkArray)
            libPathSet.add(libPath)
        }
    }

    res.add(all: allOptions)
    let binOptions = ArrayList<String>()
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, linkSort, compileTask.isLto), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, linkSort, compileTask.isLto), at: 0)
    res.add(all: binOptions)
    res.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))

    return res
}

func getForcedLibArgs(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res = ArrayList<String>()
    var cLibNameList = ArrayList<String>([compileTask.rootPkgName]) // Record dependent root packages
    var linkSort: HashSet<String> = getDepSet(compileTask.fullName, buildConfig.requiresMap,
        buildConfig.superPkgSubMap,
        isForMacroRequire: buildConfig.packageList.macros.contains(compileTask.fullName)) // Record dependent package names

    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    var binDepSet = HashSet<String>()
    let forcedLibSet: HashSet<String> = HashSet<String>(compileTask.subPkgSet)
    for (forcedLib in compileTask.subPkgSet) {
        forcedLibSet.add(all: getDepSet(forcedLib, buildConfig.requiresMap,
            buildConfig.superPkgSubMap,
            isForMacroRequire: buildConfig.packageList.macros.contains(forcedLib)))
    }
    for (depName in forcedLibSet) {
        if (linkSort.contains(depName)) {
            continue
        }
        if (buildConfig.superPkgMap.contains(compileTask.fullName) &&
            buildConfig.superPkgMap.contains(depName.split(".")[0]) &&
            compileTask.fullName != depName.split(".")[0]) {
            // combined libs have been connected already
            // sub-packages of combined libs will not be forced-linked
            continue
        }

        let depRootName: String = getRootPkgName(depName)
        if (buildConfig.packageList.macros.contains(depName) || buildConfig.packageList.test.contains(depName)) {
            continue
        }
        let libPath = Path(compileTask.targetDir).join(depRootName).toString()
        let linkArray: ArrayList<String> = forcedLinkPackage(depName, libPath, compileTask.isLto)
        if (linkArray.size <= 1) {
            allOptions.add(all: linkArray)
            binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
                isForMacroRequire: buildConfig.packageList.macros.contains(depName)))
            continue
        }
        if (libPathSet.contains(libPath)) {
            allOptions.add(linkArray[linkArray.size - 1])
        } else {
            allOptions.add(all: linkArray)
            libPathSet.add(libPath)
        }

        binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap,
            isForMacroRequire: buildConfig.packageList.macros.contains(depName)))
    }

    allOptions.add(all: getForcedBinDep(compileTask, buildConfig, binDepSet, cLibNameList))
    addForcedLibArgs(res, allOptions, compileTask.target)
    if (let Some(stdLinks) <- addStdLibsLink(compileTask.target)) {
        res.add(all: stdLinks)
    } else {
        println("Warning: cannot find std dynamic libraries for target '${compileTask.target}' " +
            "from $CANGJIE_HOME/runtime/lib while compiling combined package ${compileTask.fullName}, " +
            "output may be invalid if sub packages import std libraries")
    }

    return res
}

@When[os == "Linux" || os == "Windows"]
func getForcedBinDep(compileTask: CompileTask, buildConfig: BuildConfig, binDepSet: HashSet<String>, cLibNameList: ArrayList<String>): ArrayList<String> {
    var binOptions = ArrayList<String>()
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, binDepSet, compileTask.isLto), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, binDepSet, compileTask.isLto), at: 0)
    let cLibLinkMap = if (compileTask.isCrossCompile) {
        buildConfig.globalConfig.crossCLibLinkMap
    } else { buildConfig.globalConfig.cLibLinkMap }
    binOptions.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))
    return binOptions
}

@When[os == "macOS"]
func getForcedBinDep(compileTask: CompileTask, buildConfig: BuildConfig, binDepSet: HashSet<String>, cLibNameList: ArrayList<String>): ArrayList<String> {
    var binOptions = ArrayList<String>()

    let cLibLinkMap = if (compileTask.isCrossCompile) {
        buildConfig.globalConfig.crossCLibLinkMap
    } else { buildConfig.globalConfig.cLibLinkMap }

    if (crossCompileTarget.contains("linux") && !crossCompileTarget.contains("android")) {
        binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, binDepSet, compileTask.isLto), at: 0)
        binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, binDepSet, compileTask.isLto), at: 0)
        binOptions.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))
    } else {
        binOptions.add(all: getForcedBinDepPath(buildConfig.binDepMap, binDepSet, compileTask.isLto), at: 0)
        binOptions.add(all: getForcedBinDepPath(buildConfig.crossBinDepMap, binDepSet, compileTask.isLto), at: 0)
        let cLibLinkStr = getCLibLinkStr(cLibLinkMap, cLibNameList)

        var libName = ""
        var libPath = ""
        for (part in cLibLinkStr) {
            if (part == "-L") {
                continue
            } else if (part.startsWith("-l")) {
                libName = part.removePrefix("-l")
            } else {
                libPath = part
            }
            if (!libName.isEmpty() && !libPath.isEmpty()) {
                binOptions.add(all: forcedLinkPackage(libName, libPath, false))
                libName = ""
                libPath = ""
            }
        }
    }
    return binOptions
}

@When[os == "macOS"]
func getForcedBinDepPath(binDepMap: HashMap<String, BinDeps>, binDepSet: HashSet<String>, isLto: Bool): ArrayList<String> {
    let libPathSet = ArrayList<String>()
    for ((_, eachModuleDepMap) in binDepMap) {
        for (index in (eachModuleDepMap.packageRequiresSort.size - 1)..=0 : -1) {
            let depName = eachModuleDepMap.packageRequiresSort[index]
            if (!binDepSet.contains(depName)) {
                continue
            }
            let info = eachModuleDepMap.packageRequires[depName]
            if (info.isMacroPackage) {
                continue
            }
            let linkArray: ArrayList<String> = forcedLinkPackage(info.libName, info.libPath, isLto)
            libPathSet.add(all: linkArray)
        }
    }
    return libPathSet
}

func addForcedLibArgs(res: ArrayList<String>, allOptions: ArrayList<String>, compileTarget: String): Unit {
    if (compileTarget.contains("darwin") || compileTarget.contains("apple")) {
        var pathSet = HashSet<String>()
        var linkArr = ArrayList<String>()
        var index = 0
        var optionsNameSet = HashSet<String>()
        while (index < allOptions.size) {
            if (allOptions[index] != "-L") {
                res.add("--link-options=-force_load ${allOptions[index]}")
                index += 1
                continue
            }
            if (!pathSet.contains(allOptions[index + 1])) {
                linkArr.add(allOptions[index])
                linkArr.add(allOptions[index + 1])
                pathSet.add(allOptions[index + 1])
            }
            if (!optionsNameSet.contains(allOptions[index + 2])) {
                linkArr.add(allOptions[index + 2])
                optionsNameSet.add(allOptions[index + 2])
            }
            index += 3
        }
        res.add(all: linkArr)
    } else {
        if (!allOptions.isEmpty()) {
            res.add("--link-options=--whole-archive")
            res.add(all: allOptions)
            res.add("--link-options=--no-whole-archive")
        }
    }
}

public func getDepSet(fullName: String, requiresMap: HashMap<String, HashSet<String>>,
    superPkgSubMap: HashMap<String, String>, isForMacroRequire!: Bool = false, superPkgMap!: ?HashMap<String, SuperPackageConfig> = None): HashSet<String> {
    var linkPkgs = HashSet<String>()
    var queue = ArrayList<String>()
    queue.add(fullName)
    let currentRoot = fullName.split(".")[0]
    while (queue.size != 0) {
        var cur = queue[0]
        queue.remove(at: 0)
        if (!requiresMap.contains(cur)) {
            continue
        }
        for (k in requiresMap[cur]) {
            var finalPkg = k

            // for bin-dependencies, if lib of sub pkg does not exist,
            // but lib of its root exists, use root lib instead
            if (!isForMacroRequire && superPkgSubMap.contains(k)) {
                printSuperPkgWarn(k)
                finalPkg = superPkgSubMap[k]
            }

            // for sub packages in other combined module,
            // if current module is combined, use root lib instead
            let finalRoot = k.split(".")[0]
            if (let Some(superPkgMap) <- superPkgMap && superPkgMap.contains(finalRoot) && (finalRoot != currentRoot)) {
                linkPkgs.add(finalRoot)
                continue
            }

            if (!linkPkgs.contains(finalPkg)) {
                linkPkgs.add(finalPkg)
                queue.add(finalPkg)
            }
        }
    }
    return linkPkgs
}

public func printSuperPkgWarn(subPkgName: String): Unit {
    if (!FIXED_SUBPACKAGE.contains(subPkgName)) {
        FIXED_SUBPACKAGE.add(subPkgName)
        eprintln("Warning: binary package '${subPkgName}' may be combined by " +
            "'lib${subPkgName.split(".")[0]}${DYLIB_POSTFIX}', cjpm will use combined package instead")
    }
}

func addStdLibsLink(compileTarget: String): ?ArrayList<String> {
    let isTargetLinux = compileTarget.contains("linux")
    let cjHome = getVariable("CANGJIE_HOME") ?? return None
    let runtimePath = Path(cjHome).join("runtime").join("lib").toString()
    if (directoryExists(runtimePath)) {
        let (platform, framework) = getTargetKeywords(compileTarget)

        // get directory of std dylib
        let runtimeDirList = getDirectoryList(runtimePath)
        var runtimeLibPath = ""
        for (dirInfo in runtimeDirList) {
            if (checkRuntimeLibDir(dirInfo.name, platform, framework)) {
                runtimeLibPath = dirInfo.path.toString()
                break
            }
        }
        if (runtimeLibPath.isEmpty()) {
            return None
        }

        // get dylibs
        let libSet = HashSet<String>()
        for (fileInfo in getFileList(runtimeLibPath)) {
            libSet.add(fileInfo.name)
        }

        // add links
        if (!libSet.isEmpty()) {
            let res = ArrayList<String>()
            if (isTargetLinux) {
                res.add("--link-options=--as-needed")
            }
            for (lib in libSet) {
                res.add("-l:${lib}")
            }
            if (isTargetLinux) {
                res.add("--link-options=--no-as-needed")
            }
            return res
        }
    }
    return None
}

func getTargetKeywords(compileTarget: String): (String, String) {
    var platform = "unknown"
    var framework = "unknown"

    // check platform
    if (compileTarget.contains("darwin")) {
        platform = "darwin"
    } else if (compileTarget.contains("ohos")) {
        platform = "ohos"
    } else if (compileTarget.contains("linux")) {
        platform = "linux"
    } else if (compileTarget.contains("windows") || compileTarget.contains("w64")) {
        platform = "windows"
    }

    // check framework
    if (compileTarget.contains("aarch64") || compileTarget.contains("arm64")) {
        framework = "aarch64"
    } else if (compileTarget.contains("x86")) {
        framework = "x86"
    }

    return (platform, framework)
}

func checkRuntimeLibDir(dirName: String, platform: String, framework: String): Bool {
    if (dirName.contains("ohos") && (platform == "linux")) {
        return false
    }
    return dirName.contains(platform) && dirName.contains(framework)
}