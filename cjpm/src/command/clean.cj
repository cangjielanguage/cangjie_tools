// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*
import cjpm.config.*

// CleanCommand represents the clean command
class CleanCommand <: Handle {
    private let CLEAN_USAGE: String = """
Clean the files in the target directory.
Will not clean the configuration files "cjpm.toml" and "cjpm.lock" of the current module.

Usage:
  cjpm clean [option]

Available options:
  -g                     clean debug version in target directory
  --target-dir <value>   specify target directory
  -h, --help             help for clean
  --skip-script          disable script 'build.cj'."""

    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove clean itself
    }

    public override func printHelp(): Unit {
        println(CLEAN_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "g", ["target-dir=", "skip-script"])

        if (unparseArgs.size != 0) {
            if (unparseArgs.contains("--target-dir")) {
                eprintln("Error: the '--target-dir' option needs a value")
                return false
            }
            eprintln("Error: unknown command '${unparseArgs[0]}' for cjpm clean")
            return false
        }

        let isDebug: Bool = getBoolOption(argMap, "-g")

        if (let Some(dir) <- handleTargetDirOption(argMap)) {
            return doClean(DIR_CURRENT, dir, isDebug)
        } else {
            return false
        }
    }
}
