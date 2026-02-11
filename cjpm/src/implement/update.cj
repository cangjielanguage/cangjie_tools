// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.collection.concurrent.*
import std.fs.*
import std.sync.*
import cjpm.config.*

let UPDATE_GLOBAL_CONFIG = GlobalConfig()
let UPDATE_ANALYSIS_ENGINE = DepAnalysisEngine()
let UPDATE_INDEX_LIST = ArrayList<(String, String)>()
var UPDATE_DEPOT = Depot()

// Command update implement.
public func doUpdate(): Bool {
    // first fo all, force-update all index
    if (!updateIndex()) {
        return false
    }

    // initialize dep-analysis engine
    UPDATE_ANALYSIS_ENGINE.isTest = true
    UPDATE_ANALYSIS_ENGINE.rootModulePath = getCanonicalPath(DIR_CURRENT)

    var tomlInfo: TomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    if (!checkForWorkspace(DIR_CURRENT, tomlInfo, "")) {
        return false
    }

    let (loadFlag, lockFile) = loadLockFile(DIR_CURRENT)
    if (!loadFlag) {
        return false
    }

    if (!deleteFile(LOCK_FILE_NAME)) {
        return false
    }

    // update 'cjpm.lock' file
    var checkFlag: Bool = updateDepMap(DIR_CURRENT, "", "", None)
    if (!COMMON_INFO.inWorkspace) {
        if (!checkFlag) {
            return false
        }
    } else {
        for (mem in COMMON_INFO.members) {
            checkFlag = checkFlag && updateDepMap(mem, "", "", None)
        }
        if (!checkFlag) {
            return false
        }
    }

    // deal with repo-deps
    if (!UPDATE_ANALYSIS_ENGINE.analyseRepoDeps()) {
        return false
    }

    // toml check for repo-deps
    for ((_, finalDep) in UPDATE_ANALYSIS_ENGINE.finalDepMap) {
        if (!finalDep.sourceInfo.isCenterRepoDep()) {
            continue
        }
        let tomlInfo = loadModuleFile(finalDep.sourcePath) ?? return false
        if (tomlInfo.isWorkSpaceConfig) { // always not
            eprintln("Error: dependency '${finalDep.name}' from central repository cannot be a workspace")
            return false
        }
        if (!tomlInfo.pkg.organization.isEmpty()) {
            tomlInfo.pkg.name = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
        }
        if (!verifyModuleConfig(tomlInfo, finalDep.sourcePath, DIR_CURRENT)) {
            return false
        }
    }

    // Don't add path dependencies for 'cjpm.lock'
    let depMap = UPDATE_ANALYSIS_ENGINE.getDepMap()
    depMap.removeIf {_, v => v.path.isSome()}

    return creatLockFile(DIR_CURRENT, ModuleLock(depMap, lockFile.scripts))
}

func updateDepMap(path: String, preName: String, prePath: String, requires: ?DepInfo): Bool {
    if (!directoryExists(path)) {
        eprintln("Error: the path '${path}' does not exist")
        return false
    }

    var tomlInfo = loadModuleFile(path) ?? return false
    if (!tomlInfo.pkg.organization.isEmpty()) {
        tomlInfo.pkg.name = tomlInfo.pkg.organization + "::" + tomlInfo.pkg.name
    }
    var curModulePath: String = path

    if (let Some(depInfo) <- requires) {
        if (tomlInfo.isWorkSpaceConfig) {
            let memPath = searchModuleFromWorkspace(tomlInfo, preName, curModulePath)
            if (memPath.isEmpty()) {
                eprintln("Error: the member module '${preName}' does not exist in the workspace of '${path}'")
                return false
            }
            tomlInfo = loadModuleFile(memPath) ?? return false
            depInfo.pathInWorkspace = memPath
            curModulePath = memPath
        }
    }

    if (!verifyModuleConfig(tomlInfo, curModulePath, DIR_CURRENT)) {
        return false
    }

    if (let Some(depInfo) <- requires) {
        // verify that `requires` block that added this module is compatible with its medatada
        if (preName != tomlInfo.pkg.name) {
            eprintln("Error: the require name '${preName}' is different to name '${tomlInfo.pkg.name}'")
            eprintln("  - ${Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()}")
            eprintln("  - ${Path(curModulePath).join(CONFIG_FILE_NAME).toString()}")
            return false
        }
    } else {
        UPDATE_ANALYSIS_ENGINE.addDepInfo(tomlInfo.pkg.name, DepInfo(getCanonicalPath(curModulePath)))
    }
    UPDATE_ANALYSIS_ENGINE.addDepPath(tomlInfo.pkg.name, curModulePath)

    tomlInfo.pkg.path = curModulePath
    return updateDepModule(tomlInfo.dependencies, curModulePath, tomlInfo.pkg.name, "dependencies") &&
        updateDepModule(tomlInfo.testDependencies, curModulePath, tomlInfo.pkg.name, "test-dependencies")
}

func updateDepModule(requires: HashMap<String, DepInfo>, prePath: String, preName: String, fieldName: String): Bool {
    var updateCheckFlag = true
    for ((name, v) in requires) {
        // save temp repo-dep
        v.importedPath = Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()
        v.importedModuleName = preName
        if (let Some(version) <- v.version) {
            UPDATE_ANALYSIS_ENGINE.repoDepList.add((name, v))
            continue
        }

        // convert to absolute path for future comparisons
        if (let Some(vpath) <- v.path) {
            if (!checkEnvVars(vpath, prePath)) {
                updateCheckFlag = false
                continue
            }
            let newPath = replaceEnvVars(vpath)
            if (newPath.isEmpty()) {
                eprintln("Error: the path can't be empty which is listed in '${fieldName}' field" +
                    " at ${Path(prePath).join(CONFIG_FILE_NAME).toString()}")
                updateCheckFlag = false
                continue
            }
            let depPath: String = getPath(prePath, newPath)
            if (!directoryExists(depPath)) {
                eprintln("Error: the path '${newPath}' does not exist which is listed in '${fieldName}' field" +
                    " at ${Path(prePath).join(CONFIG_FILE_NAME).toString()}")
                return false
            }
            v.path = depPath
        }

        v.importedPath = Path(getCanonicalPath(prePath)).join(CONFIG_FILE_NAME).toString()

        if (let Some(currentDep) <- UPDATE_ANALYSIS_ENGINE.finalDepMap.get(name)) {
            let current = currentDep.sourceInfo
            let (isAlternative, errInfo) = current.isAlternative(v)
            let isGitLocalDep = checkGitLocalDep(name, current, v)
            if (!errInfo.isEmpty() && !isGitLocalDep) {
                eprintln("Error: modules with name '${name}' in dependency tree are conflicted. " +
                    "If existing dependency was updated in cjpm.toml then do `cjpm update`")
                eprintln(errInfo)
                updateCheckFlag = false
            }
            if (!errInfo.isEmpty() && isGitLocalDep) {
                continue
            }
            if (errInfo.isEmpty() && !isAlternative) {
                current.updateWith(v)
                continue
            }
        }
        UPDATE_ANALYSIS_ENGINE.addDepInfo(name, v)

        let depInfo: DepInfo = UPDATE_ANALYSIS_ENGINE.finalDepMap[name].sourceInfo
        let (flag, isOnlineDep, depPath) = getDepPath(name, depInfo, prePath, UPDATE_GLOBAL_CONFIG)
        if (!flag) {
            updateCheckFlag = false
            continue
        }

        freshGitPathDep(depInfo, UPDATE_GLOBAL_CONFIG)

        if (!updateDepMap(depPath, name, prePath, depInfo)) {
            updateCheckFlag = false
        }

        if (isOnlineDep && !UPDATE_GLOBAL_CONFIG.gitStack.pop()) {
            eprintln("Error: failed to analyse online dependencies")
            updateCheckFlag = false
        }
    }

    return updateCheckFlag
}

func checkGitLocalDep(name: String, curInfo: DepInfo, newInfo: DepInfo): Bool {
    if (!(curInfo.isGitDep() && newInfo.isPathDep()) || !UPDATE_ANALYSIS_ENGINE.finalDepMap.contains(name)) {
        return false
    }
    return UPDATE_ANALYSIS_ENGINE.finalDepMap[name].sourcePath == newInfo.path.getOrDefault({=> ""})
}

func updateIndex(): Bool {
    if (!UPDATE_DEPOT.loadConfiguration(isError: false)) {
        return true
    }
    let indexDir = Path(UPDATE_DEPOT.localRepo).join("index").toString()
    if (!directoryExists(indexDir)) {
        return true
    }

    for (orgDir in getDirectoryList(indexDir)) {
        updateIndexForOrg(orgDir, orgDir.name)
    }

    var downloadTasks = ArrayList<Future<Bool>>()
    let defaultSize = 16 // default parallel value
    let actualSize = min(defaultSize, maxParallelSize)
    let downloadSem = Semaphore(actualSize)
    let downloadMtx: Mutex = Mutex()
    let downloadErrorLogs: ConcurrentLinkedQueue<String> = ConcurrentLinkedQueue<String>()

    for ((org, name) in UPDATE_INDEX_LIST) {
        downloadSem.acquire()
        downloadTasks.add(
            spawn {
                =>
                try {
                    let (flag, output) = UPDATE_DEPOT.updateIndexWithOutput(name, if (org == DEFAULT_ORGANIZATION) { "" } else { org })
                    if (!flag) {
                        downloadErrorLogs.add(output)
                    }
                    downloadSem.release()
                    return flag
                } catch (e: Exception) { 
                    downloadErrorLogs.add(e.message) 
                    downloadSem.release()
                    return false
                }
            }
        )
    }

    var downloadFlag = true
    for (task in downloadTasks) {
        downloadFlag &&= task.get()
    }

    if (!downloadFlag) {
        logDownloadErrors(downloadErrorLogs)
    }
    return true
}

func logDownloadErrors(downloadErrorLogs: ConcurrentLinkedQueue<String>): Unit {
    var hasOpenSSLError = false
    var hasConnectError = false
    let unknownErrors = HashSet<String>() 

    eprintln("Warning: cannot update some index files due to following reason:")

    for (output in downloadErrorLogs) {
        if (output.contains("Can not load openssl library") && !hasOpenSSLError) {
            eprintln("Warning: ${output}")
            eprintln("Warning: please ensure that openssl has been correctly installed and configured according to the \"Usage Instruction\" section of the documentation")
            hasOpenSSLError = true
        } else if (output.contains("HttpEngine1#connect") && !hasConnectError) {
            eprintln("Warning: ${output}")
            eprintln("Warning: unable to connect to https server.")
            hasConnectError = true
        } else {
            unknownErrors.add(output)
        }
    }

    for (msg in unknownErrors) {
        eprintln("Warning: ${msg}")
    }
}

func updateIndexForOrg(dir: FileInfo, org: String): Unit {
    for (level1Dir in getDirectoryList(dir.path.toString())) {
        // directory of level1, size of name should be 2
        if (level1Dir.name.size != 2) {
            continue
        }
        for (level2Dir in getDirectoryList(level1Dir.path.toString())) {
            // directory of level1, size of name should be 1 or 2
            if (level2Dir.name.size > 2) {
                continue
            }
            for (indexFile in getFileList(level2Dir.path.toString())) {
                // files of index
                if (indexFile.name.startsWith(level1Dir.name + level2Dir.name)) {
                    UPDATE_INDEX_LIST.add((org, indexFile.name))
                }
            }
        }
    }
}
