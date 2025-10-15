// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import cjpm.config.*

// Use cjlint to check codes in srcDir
func buildCodeCheck(buildConfig: BuildConfig): Bool {
    if (!buildConfig.isLint) {
        return true
    }

    var tomlInfo = loadModuleFile(DIR_CURRENT) ?? return false

    if (state.isExperimental) {
        var totalflag = true
        var hasErrors = false
        for (path in tomlInfo.pkg.actualSources.allPaths) {
            // Default operation: cjlint -f srcDir
            if (buildConfig.isVerbose) {
                let cmdStr: String = "cjlint -f ${path} 2>&1"
                println("check codes in ${path}: ${cmdStr}")
            }
            let (flag, outInfo, errInfo) = execWithOutput("cjlint", ArrayList<String>(["-f", path.toString()]), spawnForScript: true)
            let output: String = errInfo + outInfo
            print(output)
            totalflag &&= flag
            hasErrors ||= output.contains("error:") || output.contains("error\u{1B}[0m:")
        }

        if (!totalflag) {
            return false
        }
        if (hasErrors) {
            eprintln("Error: code specifications with level 'MANDATORY' are violated, please check related files")
            deleteDirectory(buildConfig.globalConfig.targetDir)
            return false
        }
        return true
    } else {
        // Default operation: cjlint -f srcDir
        if (buildConfig.isVerbose) {
            let cmdStr: String = "cjlint -f ${tomlInfo.pkg.srcDir.value} 2>&1"
            println("check codes in ${tomlInfo.pkg.srcDir.value}: ${cmdStr}")
        }

        let (flag, outInfo, errInfo) = execWithOutput("cjlint", ArrayList<String>(["-f", tomlInfo.pkg.srcDir.value]), spawnForScript: true)
        let output: String = errInfo + outInfo
        print(output)
        if (!flag) {
            return false
        }
        if (output.contains("error:") || output.contains("error\u{1B}[0m:")) {
            eprintln("Error: code specifications with level 'MANDATORY' are violated, please check related files")
            deleteDirectory(buildConfig.globalConfig.targetDir)
            return false
        }

        return true
    }
}
