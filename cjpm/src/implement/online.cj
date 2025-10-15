// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.fs.*
import cjpm.config.*

func getOnlineDepPath(modulePath: String, depName: String, depInfo: DepInfo): Option<String> {
    var depPath = ""
    // used for central repository
    if (depInfo.isCenterRepoDep()) {
        let (loadFlag, sourcePath) = processRepoDependencyModule(depName, depInfo.version.getOrThrow(), modulePath)
        if (!loadFlag) {
            return Option.None
        }
        depPath = sourcePath
    } else {
        depPath = depInfo.path ?? if (let Some((path, commit, _)) <- downloadGitDep(depName, depInfo)) {
            depInfo.commitId = commit
            path
        } else {
            eprintln("Error: failed to retrieve git dependency ${depName}")
            return Option.None
        }
    }
    return getCanonicalPath(depPath)
}

func freshGitPathDep(depInfo: DepInfo, globalConfig: GlobalConfig): Unit {
    if (depInfo.isPathDep()) {
        let (url, commitId) = globalConfig.gitStack.top() ?? ("", "")
        if (!url.isEmpty() && url != "repository") {
            depInfo.path = Option.None
            depInfo.git = url
            depInfo.commitId = commitId
            depInfo.pathDepInGit = true
        }
    }
    return
}

func getDepPath(name: String, depInfo: DepInfo, prePath: String, globalConfig: GlobalConfig): (Bool, Bool, String) {
    var depPath: String = ""
    var isOnlineDep: Bool = false
    // used for central repository
    if (depInfo.isCenterRepoDep()) {
        let (loadFlag, sourcePath) = processRepoDependencyModule(name, depInfo.version.getOrThrow(), prePath)
        if (!loadFlag) {
            return (false, false, "")
        }
        depPath = sourcePath
        isOnlineDep = true
        globalConfig.gitStack.push(("repository", ""))
    } else {
        depPath = depInfo.path ?? if (let Some((path, commit, _)) <- downloadGitDep(name, depInfo)) {
            depInfo.commitId = commit
            isOnlineDep = true
            globalConfig.gitStack.push((depInfo.git.getOrDefault({=> ""}), commit))
            path
        } else {
            eprintln("Error: failed to retrieve git dependency ${name}")
            return (false, false, "")
        }
    }
    return (true, isOnlineDep, depPath)
}

func checkGitWorkspace(url: String, dependencyPath: String, targetCommit: String, gitStoragePath: Path):
    ?(String, String, Bool) {
    let tomlInfo: TomlInfo = loadModuleFile(dependencyPath) ?? return (dependencyPath, targetCommit, false)
    if (tomlInfo.isWorkSpaceConfig) {
        let workspaceGitPath: String = gitStoragePath
            .join("workspace-${url.hashCode().toString()}")
            .join(targetCommit)
            .toString()
        if (!directoryExists(workspaceGitPath)) {
            try {
                Directory.create(workspaceGitPath, recursive: true)
                rename(dependencyPath, to: workspaceGitPath, overwrite: true)
            } catch (e: Exception) {
                eprintln(e.message)
                eprintln("Error: failed to move workspace from git '${url}")
                return Option.None
            }
        }
        deleteDirectory(dependencyPath)
        if (dependencyPath.contains(".git-install-temp")) {
            deleteDirectory(getGlobalFolderPath().join("git").join(".git-install-temp").toString())
        }
        return (workspaceGitPath, targetCommit, true)
    }
    return (dependencyPath, targetCommit, false)
}

func setGitRoot(installConfig: InstallConfig): Bool {
    if (installConfig.verbose) {
        println("cjpm install: installing module from '${installConfig.gitConfig.url}'")
    }
    var gitModule: DepInfo = DepInfo()
    gitModule.git = installConfig.gitConfig.url
    if (!installConfig.gitConfig.branch.isEmpty()) {
        gitModule.branch = installConfig.gitConfig.branch
    }
    if (!installConfig.gitConfig.tag.isEmpty()) {
        gitModule.tag = installConfig.gitConfig.tag
    }
    if (!installConfig.gitConfig.commitId.isEmpty()) {
        gitModule.commitId = installConfig.gitConfig.commitId
    }
    if (let Some((path, _, isWorkspace)) <- downloadGitDep(".git-install-temp", gitModule)) {
        if (isWorkspace) {
            installConfig.globalConfig.rootPath = getCanonicalPath(path)
            installConfig.globalConfig.targetDir = getPath(installConfig.globalConfig.rootPath, TARGET)
            return true
        }
        let nameSplit = installConfig.gitConfig.url.split("/")
        let name: String = nameSplit[nameSplit.size - 1].split(".")[0]
        let newPath: String = path.replace(".git-install-temp", name)
        if (!deleteDirectory(newPath) || !createDirectory(newPath)) {
            return false
        }
        try {
            rename(path, to: newPath, overwrite: true)
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: failed to move ${path}")
            return false
        }
        if (!deleteDirectory(getGlobalFolderPath().join("git").join(".git-install-temp").toString())) {
            return false
        }
        installConfig.globalConfig.rootPath = getCanonicalPath(newPath)
        if (!fileExists(getPath(installConfig.globalConfig.rootPath, CONFIG_FILE_NAME))) {
            eprintln("Error: there is no valid cjpm config in --git ${installConfig.gitConfig.url}")
            return false
        }
        installConfig.globalConfig.targetDir = getPath(installConfig.globalConfig.rootPath, TARGET)

        return true
    }
    return false
}
