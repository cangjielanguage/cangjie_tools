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

public let fixedSubPackage = HashSet<String>()

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

    if (!buildConfig.globalConfig.isTest && buildConfig.globalConfig.isVerbose && buildConfig.exePkgList.size > 0) {
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
        if (!runScript(buildConfig.scriptConfig, BUILD_SCRIPT_STEP_POST, "build")) {
            return false
        }
        if (buildConfig.globalConfig.isInstall && !runScript(buildConfig.scriptConfig,
            BUILD_SCRIPT_STEP_POST, "install")) {
            return false
        }
    }

    return true
}

func recordCjpmBuildStartTime(tomlInfo: TomlInfo) {
    if (tomlInfo.profile.build.analysisCompilePerformance) {
        let cjpmCommand: String = String.join(getCommandLine(), delimiter:" ")
        commandCache.add(CommandInfo("cjpm", cjpmCommand))
        showCache.add(PkgInfo("cjpm", "B", DateTime.now().toUnixTimeStamp().toMilliseconds()))
        memProcess = addMemoryProcess()
    }
}

func recordCjpmBuildEndTime(tomlInfo: TomlInfo, path: String) {
    if (tomlInfo.profile.build.analysisCompilePerformance) {
        memProcess.cancel()
        memProcess.get()
        var cjpmEndTime = DateTime.now().toUnixTimeStamp().toMilliseconds()
        showCache.add(PkgInfo("cjpm", "E", cjpmEndTime))
        cjpmCompileInfo.add(cjpmEndTime.toString(),getCjpmMemInfo())
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

    if (!buildConfig.globalConfig.isSkipScript && !isScriptDep) {
        let scriptConfig: ScriptConfig = makeScriptConfigByToml(buildConfig.globalConfig.rootPath,
            buildConfig.globalConfig.rootPath, tomlInfo, buildConfig.isDebug, buildConfig.isVerbose,
            buildConfig.isCrossCompile) ?? return Option.None
        if (buildConfig.globalConfig.isInstall && !runScript(scriptConfig, BUILD_SCRIPT_STEP_PRE, "install")) {
            return Option<ModuleResolve>.None
        }
        if (!runScript(scriptConfig, BUILD_SCRIPT_STEP_PRE, "build")) {
            return Option<ModuleResolve>.None
        }
        buildConfig.scriptConfig = scriptConfig
    }

    if (!checkForWorkspace(buildConfig.globalConfig.rootPath, tomlInfo, buildConfig.memberModule,
        isDebug: buildConfig.isDebug, isTest: buildConfig.globalConfig.isTest, checkWorkspaceDir: !isScriptDep)) {
        return Option<ModuleResolve>.None
    }
    if (buildConfig.isInstall && !COMMON_INFO.inWorkspace && tomlInfo.pkg.outputType != EXE_TYPE) {
        eprintln("Error: output-type in " +
            "${Path(buildConfig.globalConfig.rootPath).join(CONFIG_FILE_NAME).toString()} " +
            "must be 'executable' for cjpm install")
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

    if (!tomlInfo.profile.build.ltoValue.isEmpty()) {
        buildConfig.isLto = supportLto()
        buildConfig.ltoValue = tomlInfo.profile.build.ltoValue
    }
    if (tomlInfo.profile.build.analysisCompilePerformance) {
        buildConfig.globalConfig.isAnalysisCompilePerformance = true
    }
    if (tomlInfo.profile.build.incremental) {
        buildConfig.isIncremental = true
    }

    var resolveData = ModuleResolve()
    let superPkgList = ArrayList<SuperPackageConfig>()
    for (pkgs in dm.readyPkgs) {
        if (let Some(v) <- dm.packageDepMap[pkgs].superPkgCfg) {
            superPkgList.add(v)
        }
        resolveData.resolves.add(dm.packageDepMap[pkgs])
    }

    resolveData.binDeps = dm.binDepMap
    resolveData.crossBinDeps = dm.crossBinDepMap
    buildConfig.binDepMap = dm.binDepMap
    buildConfig.crossBinDepMap = dm.crossBinDepMap
    buildConfig.targetMacroMods = dm.targetMacroMods
    buildConfig.targetMacroPkgs = dm.targetMacroPkgs
    buildConfig.superPkgMap = dm.superPkgMap
    buildConfig.superPkgSubMap = dm.superPkgSubMap
    return resolveData
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
        let cacheFile = Path(buildConfig.globalConfig.targetDir)
            .join(rootPkgName)
            .join("incremental-cache.json")
            .toString()
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
            if (!COMMON_INFO.inWorkspace && buildConfig.exePkgList.size == 1 && buildConfig.exePkgList[0] == pkgName &&
                v.exeName != buildConfig.exeName) {
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

        let hasSubPkgs = buildConfig.hasSubPkgs.contains(r.fullName) ||
            buildConfig.requiredForTests // in the current compilation
        if (!lastHasSubpkgsMap.contains(r.fullName) || (lastHasSubpkgsMap[r.fullName] != hasSubPkgs)) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }

        if (!lastTimeStampMap.contains(r.fullName) ||
            buildConfig.pkgCacheMap[r.fullName].timeStamp > lastTimeStampMap[r.fullName]) {
            buildConfig.rebuildList.add(r.fullName)
            continue
        }
        if (r.outputType == EXE_TYPE && !buildConfig.requiredForTests) {
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
        if (buildConfig.macroPkgList.contains(name)) {
            buildConfig.rebuildMacros.add(name)
        }
    }
}

func hasExeFile(buildConfig: BuildConfig, fullName: String): Bool {
    var exeName: String = buildConfig.exeName
    if (buildConfig.exePkgList.size > 1 || COMMON_INFO.inWorkspace) {
        exeName = fullName
    }

    if (buildConfig.isCrossCompile) {
        exeName = makeTargetExeName(exeName)
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
    }
    return
}

// Record package information in buildConfig.
func prepareBuild(buildConfig: BuildConfig, curResolveData: ModuleResolve): Unit {
    for (r in curResolveData.resolves) {
        buildConfig.allPackages.add(r.fullName)
        buildConfig.requiresMap[r.fullName] = r.requires
        if (r.isMacroPackage) {
            buildConfig.macroPkgList.add(r.fullName)
        }
        if (r.isPureTestPkg()) {
            buildConfig.testPkgs.add(r.fullName)
        }
        if (r.outputType == DYNAMIC_TYPE) {
            buildConfig.dylibPackageList.add(r.fullName)
        }
        if (r.outputType == EXE_TYPE) {
            buildConfig.exePkgList.add(r.fullName)
        }
    }

    for ((_, deps) in curResolveData.binDeps) {
        for ((fullName, info) in deps.packageRequires) {
            if (buildConfig.requiresMap.contains(fullName)) {
                continue
            }
            buildConfig.requiresMap[fullName] = info.requires
        }
    }
    for ((_, deps) in curResolveData.crossBinDeps) {
        for ((fullName, info) in deps.packageRequires) {
            if (buildConfig.requiresMap.contains(fullName)) {
                continue
            }
            buildConfig.requiresMap[fullName] = info.requires
        }
    }

    for (fullName in buildConfig.macroPkgList) {
        buildConfig.pkgRequireByMacro.add(all: getDepSet(fullName, buildConfig.requiresMap, buildConfig.superPkgSubMap))
    }

    for ((_, jsonData) in buildConfig.jsonInformation) {
        jsonData.add(all: buildConfig.getOptionCacheData())
        if (buildConfig.isLto) {
            jsonData.add(buildConfig.ltoValue)
        }
    }

    for (pkgName in buildConfig.allPackages) {
        if (buildConfig.exePkgList.contains(pkgName)) {
            continue
        }
        let parentPkg: String = getParentPkgName(pkgName)
        if (buildConfig.allPackages.contains(parentPkg) || buildConfig.requiredForTests) {
            buildConfig.hasSubPkgs.add(parentPkg)
        }
    }

    return
}

func collectCjoTime(buildConfig: BuildConfig): Unit {
    for (pkgName in buildConfig.allPackages) {
        if (buildConfig.testPkgs.contains(pkgName)) {
            continue
        }
        if (buildConfig.macroPkgList.contains(pkgName) && !crossCompileTarget.isEmpty()) {
            collectMacroCjoTime(buildConfig, pkgName)
            continue
        }
        var cjoPath = ""
        let rootPkgName: String = getRootPkgName(pkgName)
        if (!buildConfig.exePkgList.contains(pkgName) || buildConfig.requiredForTests) {
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
            if (buildConfig.testPkgs.contains(r)) {
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

    return flag
}

func addCompileCost(compilePerformanceTargetDir : String): Unit {
    sortCompileCost(compilePerformanceTargetDir)
    var showCachePath = Path(compilePerformanceTargetDir).join("time_cost.json").toString()
    var commandCachePath = Path(compilePerformanceTargetDir).join("package_command.json").toString()
    var cjpmCompileInfoPath = Path(compilePerformanceTargetDir).join("memory_cost.json").toString()
    createAndWriteFile(showCachePath, showCache.serialize().toJson().toJsonString())
    createAndWriteFile(commandCachePath, commandCache.serialize().toJson().toJsonString())
    createAndWriteFile(cjpmCompileInfoPath,cjpmCompileInfo.serialize().toJson().toJsonString())
}

func sortCompileCost(compilePerformancePath : String): Unit {
    var startMap = HashMap<String, Int64>()
    var endMap = HashMap<String, Int64>()
    var allPkgs = ArrayList<String>()
    for (k in showCache) {
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

    for (cache in showCache) {
        cache.tid = threadQueue[cache.name]
    }

    var timeCache = HashMap<String, String>()
    for (k in allPkgs) {
        timeCache[k] = ((endMap[k] - startMap[k]) * Duration.millisecond).toString()
    }
    var timeCachePath = Path(compilePerformancePath).join("time-cache.json").toString()
    createAndWriteFile(timeCachePath, timeCache.serialize().toJson().toJsonString())
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

    if (buildConfig.exePkgList.size == 1 && !COMMON_INFO.inWorkspace) {
        let fullName = buildConfig.exePkgList[0]
        let rootPkgName = getRootPkgName(fullName)
        if (cacheMap.contains(rootPkgName) && cacheMap[rootPkgName].packageInformation.contains(fullName)) {
            cacheMap[rootPkgName].packageInformation[fullName].exeName = buildConfig.exeName
        }
    }

    var cacheFlag = true
    for ((k, v) in cacheMap) {
        let cachePath = Path(buildConfig.globalConfig.targetDir).join(k)
        if (!createDirectory(cachePath.toString())) {
            cacheFlag = false
            continue
        }
        let cacheFile = cachePath.join("incremental-cache.json").toString()
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

func getCommandArgs(resolveConfig: ResolveConfig, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    var res: ArrayList<String> = ArrayList<String>()
    res.add(all: appendCommandOptions(resolveConfig, buildConfig, isNativeForCross))
    res.add(all: getDepLinkCommand(resolveConfig, buildConfig, isNativeForCross))
    if (buildConfig.customizedOption.size != 0) {
        for (k in buildConfig.customizedOption) {
            if (resolveConfig.customizedOption.contains(k)) {
                res.add(all: extractOptionByString(resolveConfig.customizedOption[k]))
            }
        }
    }
    if (!isNativeForCross) {
        if (!resolveConfig.compileOption.isEmpty()) {
            res.add(all: extractOptionByString(resolveConfig.compileOption))
        }
        if (!buildConfig.globalConfig.overrideCompileOption.isEmpty()) {
            res.add(all: extractOptionByString(buildConfig.globalConfig.overrideCompileOption))
        }
    } else {
        if (!resolveConfig.nativeCompileOption.isEmpty()) {
            res.add(all: extractOptionByString(resolveConfig.nativeCompileOption))
        }
        if (!buildConfig.globalConfig.nativeOverrideOption.isEmpty()) {
            res.add(all: extractOptionByString(buildConfig.globalConfig.nativeOverrideOption))
        }
    }
    res.add(all: appendLinkOption(resolveConfig, buildConfig.requiredForTests, isNativeForCross))
    res.add(all: getTypeCommand(resolveConfig, buildConfig, isNativeForCross))
    return res
}

func appendCommandOptions(resolveConfig: ResolveConfig, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    var res: ArrayList<String> = ArrayList<String>()
    for (p in buildConfig.globalConfig.cjPathList) {
        if (directoryExists(p)) {
            res.add("--import-path=${p}")
        }
    }
    if (buildConfig.isCrossCompile && !isNativeForCross) {
        res.add("--target=${crossCompileTarget}")
    }
    if (buildConfig.isDebug) {
        res.add("-g")
    }
    if (buildConfig.isCov) {
        res.add("--coverage")
    }
    if (!buildConfig.hasSubPkgs.contains(resolveConfig.fullName) && !buildConfig.requiredForTests) {
        res.add("--no-sub-pkg")
    }
    if (buildConfig.mockSupported) {
        res.add("--mock=on")
    }
    if (!isNativeForCross) {
        res.add("--output-dir=${resolveConfig.targetPath}")
    } else {
        res.add("--output-dir=${resolveConfig.nativePath}")
    }
    return res
}

func appendLinkOption(resolveConfig: ResolveConfig, requiredForTests: Bool, isNativeForCross: Bool): ArrayList<String> {
    if (resolveConfig.outputType == STATIC_TYPE && !requiredForTests) {
        return ArrayList<String>()
    }

    if (!isNativeForCross) {
        if (!resolveConfig.linkOption.isEmpty()) {
            return ArrayList<String>(["--link-options", resolveConfig.linkOption])
        }
    } else {
        if (!resolveConfig.nativeLinkOption.isEmpty()) {
            return ArrayList<String>(["--link-options", resolveConfig.nativeLinkOption])
        }
    }

    return ArrayList<String>()
}

func getDepLinkCommand(resolveConfig: ResolveConfig, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    let res = ArrayList<String>()
    if (resolveConfig.outputType == EXE_TYPE || resolveConfig.outputType == DYNAMIC_TYPE) {
        res.add(all: getBuildArgs(resolveConfig, buildConfig, isNativeForCross))
    }
    if (let Some(superPkgCfg) <- resolveConfig.superPkgCfg) {
        res.add(all: getForcedLibArgs(resolveConfig, superPkgCfg, buildConfig, isNativeForCross))
    }
    return res
}

func getTypeCommand(resolveConfig: ResolveConfig, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    let res = ArrayList<String>()
    if (buildConfig.globalConfig.isAnalysisCompilePerformance) {
        res.add("--profile-compile-time")
        res.add("--profile-compile-memory")
    }

    res.add(all: ["-p", resolveConfig.packagePath])

    if (buildConfig.requiredForTests && resolveConfig.hasTestFiles) {
        res.add("--export-for-test")
    }

    func recordStaticLibCommand() {
        res.add(all: makeStaticlibCommand(resolveConfig.fullName, buildConfig.isLto, buildConfig.ltoValue))
    }

    match (resolveConfig.outputType) {
        case "static" => recordStaticLibCommand()
        case "dynamic" => res.add(all: makeDylibCommand(resolveConfig.fullName, buildConfig, isNativeForCross))
        case "executable" =>
            if (buildConfig.requiredForTests) {
                // to be included in tests as a dependency, the package should be compiled as a library anyway
                recordStaticLibCommand()
                // unused main function related warnings should also be suppressed in this scenario
                res.add("-Woff=unused-main")
            } else {
                res.add(all: makeExeCommand(resolveConfig.fullName, buildConfig))
            }
        // This outputType has been checked, only the above three cases.
        case _ => ()
    }
    return res
}

@When[os == "Linux"]
func makeStaticlibCommand(fullName: String, isLto: Bool, ltoValue: String): ArrayList<String> {
    var res = ArrayList<String>()
    res.add("--output-type=staticlib")

    if (!isLto) {
        let staticlibName: String = makeCangjieStaticlibName(fullName)
        res.add("-o=${staticlibName}")
    } else {
        res.add("--lto=${ltoValue}")
        let ltoName: String = makeLtoName(fullName)
        res.add("-o=${ltoName}")
    }

    return res
}

@When[os == "Windows" || os == "macOS"]
func makeStaticlibCommand(fullName: String, _: Bool, _: String): ArrayList<String> {
    var res = ArrayList<String>()
    let staticlibName: String = makeCangjieStaticlibName(fullName)
    res.add("--output-type=staticlib")
    res.add("-o=${staticlibName}")
    return res
}

func appendOptionForBindep(binDepMap: HashMap<String, BinDeps>, linkPkgs: HashSet<String>, isLto: Bool): ArrayList<String> {
    let binOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    for ((_, eachModuleDepMap) in binDepMap) {
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
            if (linkArray.size <= 1) {
                binOptions.add(all: linkArray)
                continue
            }
            if (libPathSet.contains(info.libPath)) {
                binOptions.add(linkArray[linkArray.size - 1])
            } else {
                binOptions.add(all: linkArray)
                libPathSet.add(info.libPath)
            }
        }
    }
    return binOptions
}

func getcLibLinkStr(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>): ArrayList<String> {
    var res = ArrayList<String>()
    let cLibNameSet = HashSet<String>(cLibNameList)
    for (cLibName in cLibNameSet) {
        if (cLibLinkMap.contains(cLibName)) {
            res.add(all: cLibLinkMap[cLibName])
        }
    }
    return res
}

func makeDylibCommand(fullName: String, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    var res = ArrayList<String>()
    if (buildConfig.macroPkgList.contains(fullName)) {
        res.add("--compile-macro")
    } else {
        res.add("--output-type=dylib")
        res.add("-o=${getDylibOutputName(fullName, buildConfig, isNativeForCross)}")
    }
    return res
}

func appendCLibLinkStr(buildConfig: BuildConfig, cLibNameList: ArrayList<String>, res: ArrayList<String>, isCross: Bool): Unit {
    var cLibLinkStr = ArrayList<String>()
    if (isCross) {
        cLibLinkStr = getcLibLinkStr(buildConfig.globalConfig.crossCLibLinkMap, cLibNameList)
    } else {
        cLibLinkStr = getcLibLinkStr(buildConfig.globalConfig.cLibLinkMap, cLibNameList)
    }
    var index: Int64 = 0
    var cLibPathSet: HashSet<String> = HashSet<String>()
    var cLibNameSet: HashSet<String> = HashSet<String>()
    while (index < cLibLinkStr.size) {
        if (cLibLinkStr[index] == "-L") {
            if (!cLibPathSet.contains(cLibLinkStr[index + 1])) {
                res.add(cLibLinkStr[index])
                res.add(cLibLinkStr[index + 1])
                cLibPathSet.add(cLibLinkStr[index + 1])
            }
            index += 2
        } else if (cLibLinkStr[index].startsWith("-l")){
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
}

func getBuildArgs(resolveConfig: ResolveConfig, buildConfig: BuildConfig, isNativeForCross: Bool): ArrayList<String> {
    let res = ArrayList<String>()
    var cLibNameList = ArrayList<String>([resolveConfig.rootPkgName]) // Record dependent root packages
    var linkSort: HashSet<String> = getDepSet(resolveConfig.fullName, buildConfig.requiresMap,
        buildConfig.superPkgSubMap) // Record dependent package names

    if (linkSort.size == 0) {
        appendCLibLinkStr(buildConfig, cLibNameList, res, buildConfig.isCrossCompile && !isNativeForCross)
        return res
    }

    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    for (index in (buildConfig.allPackages.size - 1)..=0 : -1) {
        let depName = buildConfig.allPackages[index]
        if (!linkSort.contains(depName)) {
            continue
        }

        let depRootName: String = getRootPkgName(depName)
        cLibNameList.add(depRootName, at: 0)
        if (buildConfig.macroPkgList.contains(depName) || buildConfig.testPkgs.contains(depName)) {
            continue
        }
        var libPath: String
        if (!isNativeForCross) {
            libPath = Path(buildConfig.globalConfig.targetDir).join(depRootName).toString()
        } else {
            libPath = Path(buildConfig.globalConfig.nativeDir).join(depRootName).toString()
        }
        let linkArray: ArrayList<String> = linkPackage(depName, libPath, buildConfig.isLto)
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
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, linkSort, buildConfig.isLto), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, linkSort, buildConfig.isLto), at: 0)
    res.add(all: binOptions)
    appendCLibLinkStr(buildConfig, cLibNameList, res, buildConfig.isCrossCompile && !isNativeForCross)

    return res
}

func getForcedLibArgs(resolveConfig: ResolveConfig, superPkgCfg: SuperPackageConfig, buildConfig: BuildConfig,
    isNativeForCross: Bool): ArrayList<String> {
    let res = ArrayList<String>()
    var cLibNameList = ArrayList<String>([resolveConfig.rootPkgName]) // Record dependent root packages
    var linkSort: HashSet<String> = getDepSet(resolveConfig.fullName, buildConfig.requiresMap,
        buildConfig.superPkgSubMap) // Record dependent package names

    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    var binDepSet = HashSet<String>()
    let forcedLibSet: HashSet<String> = HashSet<String>(superPkgCfg.subPkgSet)
    for (forcedLib in superPkgCfg.subPkgSet) {
        forcedLibSet.add(all: getDepSet(forcedLib, buildConfig.requiresMap,
            buildConfig.superPkgSubMap))
    }
    for (depName in forcedLibSet) {
        if (linkSort.contains(depName)) {
            continue
        }
        if (buildConfig.superPkgMap.contains(resolveConfig.fullName) &&
            buildConfig.superPkgMap.contains(depName.split(".")[0]) &&
            resolveConfig.fullName != depName.split(".")[0]) {
            // combined libs have been connected already
            // sub-packages of combined libs will not be forced-linked
            continue
        }

        let depRootName: String = getRootPkgName(depName)
        if (buildConfig.macroPkgList.contains(depName) || buildConfig.testPkgs.contains(depName)) {
            continue
        }
        var libPath: String
        if (!isNativeForCross) {
            libPath = Path(buildConfig.globalConfig.targetDir).join(depRootName).toString()
        } else {
            libPath = Path(buildConfig.globalConfig.nativeDir).join(depRootName).toString()
        }
        let linkArray: ArrayList<String> = forcedLinkPackage(depName, libPath, buildConfig.isLto)
        if (linkArray.size <= 1) {
            allOptions.add(all: linkArray)
            binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap))
            continue
        }
        if (libPathSet.contains(libPath)) {
            allOptions.add(linkArray[linkArray.size - 1])
        } else {
            allOptions.add(all: linkArray)
            libPathSet.add(libPath)
        }

        binDepSet.add(all: getDepSet(depName, buildConfig.requiresMap, buildConfig.superPkgSubMap))
    }

    allOptions.add(all: getForcedBinDep(buildConfig, binDepSet, cLibNameList, isNativeForCross))

    addForcedLibArgs(res, allOptions)

    var compileTarget: String = targetConfigName
    if (!crossCompileTarget.isEmpty() && !isNativeForCross) {
        compileTarget = crossCompileTarget
    }
    if (let Some(stdLinks) <- addStdLibsLink(compileTarget)) {
        res.add(all: stdLinks)
    } else {
        println("Warning: cannot find std dynamic libraries for target '${compileTarget}' " +
            "from $CANGJIE_HOME/runtime/lib while compiling combined package ${resolveConfig.fullName}, " +
            "output may be invalid if sub packages import std libraries")
    }

    return res
}

@When[os == "Linux" || os == "Windows"]
func getForcedBinDep(buildConfig: BuildConfig, binDepSet: HashSet<String>, cLibNameList: ArrayList<String>,
    isNativeForCross:Bool): ArrayList<String> {
    var binOptions = ArrayList<String>()
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, binDepSet, buildConfig.isLto), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, binDepSet, buildConfig.isLto), at: 0)
    appendCLibLinkStr(buildConfig, cLibNameList, binOptions, buildConfig.isCrossCompile && !isNativeForCross)
    return binOptions
}

@When[os == "macOS"]
func getForcedBinDep(buildConfig: BuildConfig, binDepSet: HashSet<String>, cLibNameList: ArrayList<String>,
    isNativeForCross: Bool): ArrayList<String> {
    var binOptions = ArrayList<String>()
    if (crossCompileTarget.contains("ohos")) {
        binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, binDepSet, buildConfig.isLto), at: 0)
        binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, binDepSet, buildConfig.isLto), at: 0)
        appendCLibLinkStr(buildConfig, cLibNameList, binOptions, buildConfig.isCrossCompile && !isNativeForCross)
    } else {
        binOptions.add(all: getForcedBinDepPath(buildConfig.binDepMap, binDepSet, buildConfig.isLto), at: 0)
        binOptions.add(all: getForcedBinDepPath(buildConfig.crossBinDepMap, binDepSet, buildConfig.isLto), at: 0)
        var cLibLinkStr = ArrayList<String>()
        if (isNativeForCross) {
            cLibLinkStr = getcLibLinkStr(buildConfig.globalConfig.crossCLibLinkMap, cLibNameList)
        } else {
            cLibLinkStr = getcLibLinkStr(buildConfig.globalConfig.cLibLinkMap, cLibNameList)
        }
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

@When[os == "Linux" || os == "Windows"]
func addForcedLibArgs(res: ArrayList<String>, allOptions: ArrayList<String>): Unit {
    if (!allOptions.isEmpty()) {
        res.add("--link-options=--whole-archive")
        res.add(all: allOptions)
        res.add("--link-options=--no-whole-archive")
    }
}

@When[os == "macOS"]
func addForcedLibArgs(res: ArrayList<String>, allOptions: ArrayList<String>): Unit {
    if (crossCompileTarget.contains("ohos")) {
        if (!allOptions.isEmpty()) {
            res.add("--link-options=--whole-archive")
            res.add(all: allOptions)
            res.add("--link-options=--no-whole-archive")
        }
    } else {
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
    }
}

func getDylibOutputName(fullName: String, buildConfig: BuildConfig, isNativeForCross: Bool): String {
    if (buildConfig.isCrossCompile && !isNativeForCross) {
        return makeTargetDylibName(fullName)
    }
    return makeDylibName(fullName)
}

public func getDepSet(fullName: String, requiresMap: HashMap<String, HashSet<String>>,
    superPkgSubMap: HashMap<String, String>): HashSet<String> {
    var linkPkgs = HashSet<String>()
    var queue = ArrayList<String>()
    queue.add(fullName)
    while (queue.size != 0) {
        var cur = queue[0]
        queue.remove(at: 0)
        if (!requiresMap.contains(cur)) {
            continue
        }
        for (k in requiresMap[cur]) {
            var finalPkg = k
            if (superPkgSubMap.contains(k)) {
                printSuperPkgWarn(k)
                finalPkg = superPkgSubMap[k]
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
    if (!fixedSubPackage.contains(subPkgName)) {
        fixedSubPackage.add(subPkgName)
        eprintln("Warning: binary package '${subPkgName}' may be combined by " +
            "'lib${subPkgName.split(".")[0]}${DYLIB_POSTFIX}', cjpm will use combined package instead")
    }
}

func makeExeCommand(fullName: String, buildConfig: BuildConfig): ArrayList<String> {
    var res = ArrayList<String>()
    if (buildConfig.isLto) {
        res.add("--lto=${buildConfig.ltoValue}")
    }
    res.add("--output-type=exe")
    res.add("-o=${getExeOutputName(fullName, buildConfig)}")
    return res
}

func getExeOutputName(fullName: String, buildConfig: BuildConfig): String {
    var name: String = buildConfig.exeName

    if (buildConfig.exePkgList.size > 1 || COMMON_INFO.inWorkspace) {
        name = fullName
    }

    if (buildConfig.isCrossCompile) {
        return makeTargetExeName(name)
    }

    return makeExeName(name)
}

func addStdLibsLink(compileTarget: String): ?ArrayList<String> {
    let cjHome = getVariable("CANGJIE_HOME") ?? return None
    let runtimePath = Path(cjHome).join("runtime").join("lib").toString()
    if (directoryExists(runtimePath)) {
        let (platform, framework) = getTargetKeywords(compileTarget)

        // get directory of std dylib
        let runtimeDirList = getDirectoryList(runtimePath)
        var runtimeLibPath = ""
        for (dirInfo in runtimeDirList) {
            if (dirInfo.name.contains(platform) && dirInfo.name.contains(framework)) {
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
            res.add("--link-options=--as-needed")
            for (lib in libSet) {
                res.add("-l:${lib}")
            }
            res.add("--link-options=--no-as-needed")
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
    } else if (compileTarget.contains("linux") || compileTarget.contains("ohos")) {
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