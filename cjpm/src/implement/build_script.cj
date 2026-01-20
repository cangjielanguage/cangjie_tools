// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import std.env.*
import cjpm.config.*
import cjpm.util.Result

public func runScript(scriptConfig: ScriptConfig, step: BuildScriptStep, command: String): Bool {
    if (scriptConfig.isEmpty()) {
        return true
    }

    if (step == Pre && !scriptConfig.isRelocated && !buildScriptExe(scriptConfig)) {
        return false
    }

    let commandArg = "${step}-${command}"

    return runScriptExe(scriptConfig, commandArg)
}

public func makeScriptConfigByToml(rootPath: String, modulePath: String, tomlInfo: TomlInfo, debug: Bool,
    verbose: Bool, cross: Bool): Option<ScriptConfig> {
    var scriptConfig: ScriptConfig = ScriptConfig()
    if (!fileExists(Path(modulePath).join(BUILD_SCRIPT_NAME))) {
        if (fileExists(Path(modulePath).join(BUILD_SCRIPT_NAME).toString() + ".bak")) {
            scriptConfig.isRelocated = true
        } else {
            return scriptConfig
        }
    }

    let (flag, errlog) = tomlInfo.isCompatible()
    if (!flag) {
        eprintln("Error: ${errlog} at ${Path(modulePath).join(CONFIG_FILE_NAME).toString()}")
        return Option.None
    }
    var moduleName: String = WORKSPACE_NAME
    if (!tomlInfo.pkg.organization.isEmpty() && !tomlInfo.pkg.name.contains("::")) {
        tomlInfo.pkg.name = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
    }

    if (tomlInfo.isPkgConfig) {
        if (tomlInfo.pkg.name.contains("::")) {
            if (!organizationCheck(tomlInfo.pkg.organization, Path(modulePath).join(CONFIG_FILE_NAME).toString()) &&
                !nameCheck(tomlInfo.pkg.name.removePrefix(tomlInfo.pkg.organization + "::"),
                    Path(modulePath).join(CONFIG_FILE_NAME).toString())) {
                return Option.None
            }
            moduleName = tomlInfo.pkg.name.split("::")[1]
        } else {
            if (!nameCheck(tomlInfo.pkg.name, Path(modulePath).join(CONFIG_FILE_NAME).toString())) {
                return Option.None
            }
            moduleName = tomlInfo.pkg.name
        }
    }

    // targetDirectory is prepared here
    scriptConfig.isVerbose = verbose
    scriptConfig.moduleName = moduleName
    scriptConfig.modulePath = getCanonicalPath(modulePath)
    var srcDir = tomlInfo.pkg.srcDir.value
    scriptConfig.moduleSrcPath = getPath(scriptConfig.modulePath, replaceEnvVars(srcDir))
    scriptConfig.scriptPath = Path(scriptConfig.modulePath).join(BUILD_SCRIPT_NAME).toString()

    var targetPath: Path = Path(getCanonicalPath(rootPath)).join(BUILD_SCRIPT_DIR)
    var targetName: String = targetConfigName
    if (cross) {
        targetPath = targetPath.join(crossCompileTarget)
        targetName = crossCompileTarget
        scriptConfig.isCrossCompile = true
    }
    if (debug) {
        targetPath = targetPath.join("debug")
        scriptConfig.isDebug = true
    } else {
        targetPath = targetPath.join("release")
    }
    scriptConfig.scriptDep = tomlInfo.getBuildDependencies(modulePath, targetName, debug) ?? return Option.None
    if (!tomlInfo.isPkgConfig) {
        scriptConfig.scriptTargetDir = targetPath.join(moduleName).toString()
    } else {
        scriptConfig.scriptTargetDir = targetPath.join(swapOrgName(tomlInfo.pkg.name)).toString()
    }

    scriptConfig.scriptExe = Path(scriptConfig.scriptTargetDir).join("bin").join(BUILD_SCRIPT_OUTPUT).toString()
    scriptConfig.scriptHash = Path(scriptConfig.scriptTargetDir).join("bin").join(BUILD_SCRIPT_CACHE).toString()
    scriptConfig.scriptLog = Path(scriptConfig.scriptTargetDir).join("bin").join(BUILD_SCRIPT_LOG).toString()

    return scriptConfig
}

public func makeScriptConfig(modulePath: String, debug: Bool, verbose: Bool, cross: Bool): Option<ScriptConfig> {
    let absoluteModulePath: String = getCanonicalPath(modulePath)
    if (!safeCheck(absoluteModulePath, modulePath)) {
        return Option.None
    }

    if (!fileExists(Path(modulePath).join(BUILD_SCRIPT_NAME)) &&
        !fileExists(Path(modulePath).join(BUILD_SCRIPT_NAME).toString() + ".bak")) {
        return ScriptConfig()
    }

    let tomlInfo: TomlInfo = loadModuleFile(modulePath) ?? return Option.None
    let configScriptFileDir = getScriptDirValue(tomlInfo)
    var scriptPath: String = getScriptDirectory(DIR_CURRENT, modulePath, configScriptFileDir, true) ??
        return ScriptConfig()

    return makeScriptConfigByToml(scriptPath, modulePath, tomlInfo, debug, verbose, cross)
}

func runScriptExe(scriptConfig: ScriptConfig, commandArg: String): Bool {
    var cmd = ""
    var exePath = ""
    var envs = HashMap<String, String>()
    if (RUN_SCRIPT_CMD_MAP.contains(scriptConfig.moduleName)) {
        (cmd, exePath, envs) = RUN_SCRIPT_CMD_MAP[scriptConfig.moduleName]
        if (scriptConfig.isVerbose) {
            println(cmd)
        }
    } else {
        exePath = scriptConfig.scriptExe
        envs = getEnvMap(scriptConfig.scriptLdPath)
        if (scriptConfig.isVerbose) {
            cmd = "Run build script: ${getCmdStr(envs, exePath, ArrayList<String>([commandArg]))}"
 	        println(cmd)
        }
        RUN_SCRIPT_CMD_MAP.add(scriptConfig.moduleName, (cmd, exePath, envs))
    }
    let (flag, outInfo, errInfo) = execWithOutput(exePath, ArrayList<String>([commandArg]), envMap: envs,
        workingDirectory: Path(scriptConfig.modulePath), spawnForScript: true)
    let output: String = errInfo + outInfo
    try {
        File.appendTo(scriptConfig.scriptLog, output.toArray())
    } catch (_: Exception) {
        eprintln("Error: failed to generate log file ${scriptConfig.scriptLog}")
        return false
    }
    if (!flag) {
        eprintln(errInfo)
        eprintln("Error: failed to run build script in path ${scriptConfig.modulePath} by operation " +
            "'${commandArg}', output is redirected into ${scriptConfig.scriptLog}")
        return false
    }

    return true
}

public func runMultiScriptPost(globalConfig: GlobalConfig): Bool {
    globalConfig.scriptList.reverse()
    for (scriptConfig in globalConfig.scriptList) {
        if (!runScript(scriptConfig, Post, globalConfig.resolveType)) {
            return false
        }
    }
    return true
}

func getDepPath(modulePath: String, depName: String, depInfo: DepInfo): Option<String> {
    if (let Some(depPath) <- depInfo.path) {
        return getPath(modulePath, depPath)
    } else if (let Some(version) <- depInfo.version) {
        return "${modulePath}${SCRIPT_DEPENDENCIES_MARK}${depName}-${version}"
    } else {
        return if (let Some((path, commit, _)) <- downloadGitDep(depName, depInfo)) {
            depInfo.commitId = commit
            getCanonicalPath(path)
        } else {
            eprintln("Error: failed to retrieve git dependency ${depName}")
            Option.None
        }
    }
}

func getScriptDeps(scriptConfig: ScriptConfig): HashMap<String, DepInfo> {
    let deps = scriptConfig.scriptDep
    let (loadFlag, lockFile) = loadLockFile(scriptConfig.modulePath)
    if (!loadFlag) {
        return deps
    }

    for ((depName, depInfo) in lockFile.scripts) {
        if (deps.contains(depName)) {
            deps.add(depName, depInfo)
        }
    }
    return deps
}

func writeScriptLock(rootPath: String, scriptDeps: HashMap<String, DepInfo>): Bool {
    if (scriptDeps.isEmpty()) {
        return true
    }

    let (loadFlag, lockFile) = loadLockFile(rootPath)
    if (!loadFlag) {
        return false
    }

    scriptDeps.removeIf {_, v => v.path.isSome()}
    var lockFilePath: String = Path(rootPath).join(LOCK_FILE_NAME).toString()
    // Overwrite file only if we have meaningful changes in case field serialization order got changed
    if (lockFile.scripts != scriptDeps || !fileExists(lockFilePath)) {
        if (!lockFile.scripts.isEmpty() && !deleteFile(lockFilePath)) {
            return false
        }

        if (!creatLockFile(rootPath, ModuleLock(lockFile.requires, scriptDeps))) {
            return false
        }
    }

    return true
}

func buildScriptExe(scriptConfig: ScriptConfig): Bool {
    let crossCompileBackup: String = crossCompileTarget
    crossCompileTarget = ""

    let buildConfig = BuildConfig()
    let scriptDeps = getScriptDeps(scriptConfig)
    if (!scriptDeps.isEmpty()) {
        var depPathList: ArrayList<String> = ArrayList<String>()
        for ((depName, depInfo) in scriptDeps) {
            let depPath: String = getDepPath(scriptConfig.modulePath, depName, depInfo) ?? return false
            depPathList.add(depPath)
        }

        buildConfig.isVerbose = scriptConfig.isVerbose
        buildConfig.isIncremental = true
        buildConfig.isDebug = scriptConfig.isDebug
        buildConfig.targetDir = Path(scriptConfig.scriptTargetDir).join("libs").toString()

        buildConfig.globalConfig.isVerbose = buildConfig.isVerbose
        buildConfig.globalConfig.isIncremental = true
        buildConfig.globalConfig.rootPath = scriptConfig.modulePath
        buildConfig.globalConfig.resolveType = RESOLVE_BUILD

        let tomlInfo: TomlInfo = TomlInfo()
        tomlInfo.isWorkSpaceConfig = true
        tomlInfo.isPkgConfig = false
        tomlInfo.workspace.members.add(all: depPathList)

        let commonInfoCache: CommonInfo = COMMON_INFO
        COMMON_INFO = CommonInfo()
        if (!doBuild(tomlInfo, buildConfig, isScriptDep: true)) {
            return false
        }
        COMMON_INFO = commonInfoCache
        scriptConfig.scriptLdPath.add(all: buildConfig.globalConfig.ldPath)
    }

    crossCompileTarget = crossCompileBackup

    if (!makeScriptExe(scriptConfig, buildConfig)) {
        return false
    }

    // change name of build.cj
    if (checkParentDir(scriptConfig.moduleSrcPath, scriptConfig.scriptPath) && !relocateScript(scriptConfig)) {
        return false
    }

    return writeScriptLock(scriptConfig.modulePath, scriptDeps)
}

func relocateScript(scriptConfig: ScriptConfig): Bool {
    try {
        rename(scriptConfig.scriptPath, to: scriptConfig.scriptPath.removeSuffix("build.cj") + "build.cj.bak",
            overwrite: true)
        RELOCATED_SCRIPT_MAP.add(scriptConfig.scriptPath)
        scriptConfig.isRelocated = true
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: failed to backup build.cj")
        return false
    }

    return true
}

func getScriptDepLink(modulePath: String, buildConfig: BuildConfig): Option<ArrayList<String>> {
    let res = scanDependency(Path(modulePath))
        .inspectErr { e => eprintln(e) }
        .ok ?? return None

    var allPackageSet: HashSet<String> = HashSet<String>(buildConfig.packageList.all)
    var requires: HashSet<String> = HashSet<String>()
    for (fullName in res.dependencies) {
        for (importInfo in fullName.importsList) {
            if (!importInfo.fileImported.endsWith(PATH_SPLIT_CHAR + BUILD_SCRIPT_NAME)) {
                continue
            }
            if (allPackageSet.contains(fullName.orPkgName)) {
                requires.add(fullName.orPkgName)
            } else if (allPackageSet.contains(fullName.packageName)) {
                requires.add(fullName.packageName)
            } else {
                eprintln("Error: dependency ${fullName.packageName} is not compiled")
                return Option.None
            }
        }
    }

    var stdRequires: HashSet<String> = HashSet<String>()
    for (fullName in res.stdDependencies) {
        for (importInfo in fullName.importsList) {
            stdRequires.add(fullName.packageName)
        }
    }

    let target = if (buildConfig.isCrossCompile) { targetConfigName } else { crossCompileTarget }
    let requireTasks = requires.iterator().map { it => TaskIdentifier(it, target, "") } |> collectHashSet
    let taskId = TaskIdentifier("build-script", target, "")
    let compileTask: CompileTask = CompileTask(
        id: taskId,
        fullName: "build-script",
        targetDir: buildConfig.globalConfig.targetDir,
        isCrossCompile: buildConfig.isCrossCompile,
        target: target,
        outputType: Exe,
        requireTasks: requireTasks,
        isDebug: buildConfig.isDebug,
        isCov: buildConfig.isCov,
        mockSupported: buildConfig.mockSupported,
    )
    buildConfig.requiresMap.add(compileTask.fullName, requires)
    return getDepLinkCommand(compileTask, buildConfig)
}

func getEnvMap(ldPath: HashSet<String>): HashMap<String, String> {
    let envBuilder = EnvironmentBuilder()
    envBuilder.prepend(LD_PATH, ldPath)
    envBuilder.prepend(LD_PATH, getExtraRuntimePaths())
    return envBuilder.asCliStrings(getVariables())
}

func getCompileArgs(scriptConfig: ScriptConfig, buildConfig: BuildConfig): Option<(HashMap<String, String>, ArrayList<String>)> {
    var cmd: String = "\"${scriptConfig.scriptPath}\""
    for (p in buildConfig.globalConfig.cjPathList) {
        if (directoryExists(p)) {
            cmd += " --import-path=${p}"
        }
    }
    cmd += " -o ${scriptConfig.scriptExe}"
    if (scriptConfig.isCrossCompile) {
        cmd += " --cfg=\"target=${crossCompileTarget}\""
    } else {
        cmd += " --cfg=\"target=${targetConfigName}\""
    }
    var depLinkList: ArrayList<String> = getScriptDepLink(scriptConfig.modulePath, buildConfig) ?? return Option.None
    if (!depLinkList.isEmpty()) {
        cmd += " ${String.join(depLinkList.toArray(), delimiter: " ")} "
    }
    let env = getEnvMap(scriptConfig.scriptLdPath)
 	let cmdArgs = extractOptionByString(cmd)
    if (scriptConfig.isVerbose) {
        println("Compiling build script of `${scriptConfig.moduleName}`: ${getCmdStr(env, COMPILE_TOOL, cmdArgs)}")
    }

    return (env, cmdArgs)
}

func getScriptIncremental(scriptConfig: ScriptConfig): String {
    var res: String = ""
    if (!directoryExists(scriptConfig.scriptTargetDir) || !fileExists(scriptConfig.scriptExe)) {
        return ""
    }
    try {
        if (fileExists(scriptConfig.scriptPath)) {
            res += String.fromUtf8(File.readFrom(scriptConfig.scriptPath))
        } else if (fileExists(scriptConfig.scriptPath + ".bak")) {
            res += String.fromUtf8(File.readFrom(scriptConfig.scriptPath + ".bak"))
        }
        res += FileInfo(scriptConfig.scriptExe).lastModificationTime.toUnixTimeStamp().toSeconds().toString()
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Warning: file error in ${scriptConfig.scriptTargetDir}, cannot generate hash code, rebuild build.cj")
        return ""
    }
    return res.hashCode().toString()
}

func isRebuild(scriptConfig: ScriptConfig, buildConfig: BuildConfig): Bool {
    if (buildConfig.isRebuild) {
        return true
    }

    if (!fileExists(scriptConfig.scriptHash)) {
        return true
    }

    let preHash = String.fromUtf8(File.readFrom(scriptConfig.scriptHash))
    let curHash: String = getScriptIncremental(scriptConfig)

    return preHash != curHash
}

func makeScriptExe(scriptConfig: ScriptConfig, buildConfig: BuildConfig): Bool {
    if (!isRebuild(scriptConfig, buildConfig)) {
        return true
    }

    if (!createDirectory(Path(scriptConfig.scriptTargetDir).join("bin").toString())) {
        return false
    }

    let (envs, args) = getCompileArgs(scriptConfig, buildConfig) ?? return false
    let (flag, outInfo, errInfo) = execWithOutput(COMPILE_TOOL, args, envMap: envs, spawnForScript: true)

    if (!outInfo.isEmpty()) {
        println(outInfo)
    }
    if (!flag) {
        eprintln(errInfo)
        eprintln("Error: failed to build script ${scriptConfig.scriptPath}")
        return false
    }

    try {
        File.writeTo(scriptConfig.scriptHash, getScriptIncremental(scriptConfig).toArray())
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: failed to generate ${BUILD_SCRIPT_CACHE}")
        return false
    }

    return true
}
