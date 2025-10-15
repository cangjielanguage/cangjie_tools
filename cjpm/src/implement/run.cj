// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*
import std.env.getVariables

public func doRun(runConfig: RunConfig): Bool {
    if (!getOriginDirForRun(runConfig)) {
        return false
    }

    let binPath = Path(runConfig.targetPath).join(BIN).toString()
    runConfig.filePath = Path(binPath).join(makeExeName(runConfig.name)).toString()
    if (!fileExists(runConfig.filePath)) {
        eprintln("Error: can not find the '${runConfig.filePath}' file")
        return false
    }
    runConfig.filePath = getCanonicalPath(runConfig.filePath)

    return startRun(runConfig)
}

func getOriginDirForRun(runConfig: RunConfig): Bool {
    let tomlInfo: TomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    if (!verifyEnvConfig(tomlInfo.profile.run.envConfig, "run", Path(DIR_CURRENT).join(CONFIG_FILE_NAME).toString())) {
        return false
    }
    if (runConfig.targetPath.isEmpty()) {
        let configPath: String = Path(DIR_CURRENT).join(CONFIG_FILE_NAME).toString()
        if (!setTargetPathForRun(runConfig, tomlInfo, configPath)) {
            return false
        }
    }
    runConfig.originTargetPath = runConfig.targetPath

    if (!runConfig.isDebug) {
        runConfig.targetPath = Path(runConfig.targetPath).join("release").toString()
    } else {
        runConfig.targetPath = Path(runConfig.targetPath).join("debug").toString()
    }

    if (tomlInfo.isWorkSpaceConfig) {
        runConfig.rootPkgName = WORKSPACE_NAME
    } else {
        runConfig.rootPkgName = tomlInfo.pkg.name
    }

    runConfig.envConfig = tomlInfo.profile.run.envConfig
    return true
}

func setTargetPathForRun(runConfig: RunConfig, tomlInfo: TomlInfo, configPath: String): Bool {
    if (COMMON_INFO.inWorkspace) {
        if (COMMON_INFO.targetDir.isEmpty()) {
            runConfig.targetPath = TARGET
        } else {
            if (!safeCheck(COMMON_INFO.targetDir, configPath)) {
                return false
            }
            runConfig.targetPath = COMMON_INFO.targetDir
        }
    } else {
        if (tomlInfo.pkg.targetDir.isEmpty()) {
            runConfig.targetPath = TARGET
        } else {
            if (!replaceEnvforTargetDir(tomlInfo, configPath)) {
                return false
            }
            if (!safeCheck(tomlInfo.pkg.targetDir, configPath)) {
                return false
            }
            runConfig.targetPath = tomlInfo.pkg.targetDir
        }
    }
    return true
}

func startRun(runConfig: RunConfig): Bool {
    let envBuilder = EnvironmentBuilder.fromEnvConfig(runConfig.envConfig)
    addLibEnvConfig(envBuilder, runConfig.targetPath, runConfig.globalConfig.nativeDir, runConfig.originTargetPath)

    let (exeTool, args) = getExecPair(runConfig.filePath, runConfig.targetPath, runConfig.originTargetPath)
    if (!runConfig.runArgs.isEmpty()) {
        args.add(all: extractOptionByString(runConfig.runArgs))
    }

    let env = getVariables()
    if (runConfig.isVerbose) {
        println("Run current project: ${getCmdStr(envBuilder.asCliStrings(env), exeTool, args)}")
    }

    execAndToTerminal(exeTool, args.toArray(), envBuilder: envBuilder, originalEnv: env)
    return true
}

func getCmdStr(envMap: HashMap<String, String>, exeTool: String, args: ArrayList<String>): String {
    let envArgStr: String = envMapToString(envMap)
    var envPatch: String = ""
    var res: String = ""
    if (!envArgStr.isEmpty()) {
        envPatch = ENV_PATCH
        res = envArgStr
    }
    res += "${exeTool}"
    if (!args.isEmpty()) {
        res += " ${String.join(args.toArray(), delimiter: " ")}"
    }
    res += envPatch

    return res
}

func addLibEnvConfig(envBuilder: EnvironmentBuilder, targetDir: String, nativeDir: String, originPath: String): Unit {
    let historyFile = Path(originPath).join(HISTORY_FILE_NAME).toString()
    if (!fileExists(historyFile)) {
        return
    }
    let resInfo: ModuleResolve = loadHistoryFile(historyFile) ?? ModuleResolve()
    let ldPath = getLdPathConfig(targetDir, nativeDir, resInfo)
    envBuilder.prepend(LD_PATH, ldPath)
}

func getExecPair(filePath: String, _: String, _: String): (String, ArrayList<String>) {
    return (filePath, ArrayList<String>())
}
