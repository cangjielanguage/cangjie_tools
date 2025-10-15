// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.fs.*
import cjpm.config.*

// used for central repository
func processRepoDependencyModule(name: String, version: String, modulePath: String): (Bool, String) {
    if (version.isEmpty()) {
        eprintln("Error: the version can't be empty which is listed in 'dependencies' field " +
            "at ${Path(modulePath).join(CONFIG_FILE_NAME).toString()}")
        return (false, "")
    }
    let (result, output) = loadModuleFromRepo(name, version)
    if (!result) {
        if (!output.isEmpty()) {
            eprintln("Error: ${output}")
        }
        return (false, "")
    }
    return (true, output)
}

// used for central repository
func loadModuleFromRepo(name: String, version: String): (Bool, String) {
    try {
        if (let Some(depot) <- Depot.new()) {
            let dependencyPath = Path(depot.getLocalRepo()).join("${name}-${version}").toString()
            if (directoryExists(dependencyPath)) {
                return (true, dependencyPath)
            }

            let (result, output) = depot.download(name, version)
            if (!result) {
                return (false, output)
            }
            let p = getPath(depot.getLocalRepo(), "${name}-${version}")
            return (true, p)
        } else {
            return (false, "download error")
        }
    } catch (e: Exception) {
        return (false, "download ${name} exception")
    }
}

// used for central repository
public func setRepoRoot(installConfig: InstallConfig, artifact: String): Bool {
    if (installConfig.verbose) {
        println("cjpm install: installing ${artifact} from central repository")
    }
    let params = artifact.split(":")
    try {
        installConfig.globalConfig.rootPath = Path(DEFAULT_CONFIG_LOCATION)
            .join(".cjpm")
            .join("repository")
            .join("${params[0]}-${params[1]}")
            .toString()
        if (directoryExists(installConfig.globalConfig.rootPath)) {
            if (installConfig.verbose) {
                println("cjpm install skip downloading ${artifact}: local files exist")
            }
            installConfig.globalConfig.targetDir = getPath(installConfig.globalConfig.rootPath, TARGET)
            return true
        }
        if (let Some(depot) <- Depot.new()) {
            let (result, output) = depot.download(params[0], params[1])
            if (!result) {
                eprintln("Error: ${output}")
                return false
            }
            if (installConfig.verbose) {
                println(output)
            }
            installConfig.globalConfig.targetDir = getPath(installConfig.globalConfig.rootPath, TARGET)
            return true
        } else {
            return false
        }
    } catch (e: Exception) {
        println("Error: load module failed with network connection error")
        return false
    }
}
