// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*

let UPDATE_SHORT_OPTION: String = ""
let UPDATE_LONG_OPTION: Array<String> = ["skip-script"]

// UpdateCommand represents the update command
class UpdateCommand <: Handle {
    private let UPDATE_USAGE: String = """
Update the cjpm.lock.

Usage:
  cjpm update [option]

Available options:
  -h, --help        help for update
  --skip-script     disable script 'build.cj'."""

    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove update itself
        this.shortOptionSpec = UPDATE_SHORT_OPTION
        this.longOptionSpecs = UPDATE_LONG_OPTION
    }

    public override func printHelp(): Unit {
        println(UPDATE_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        if (!parseSkipScript(this.arguments)) {
            return false
        }

        return doUpdate()
    }
}

public func parseSkipScript(arguments: Array<String>): Bool {
    let (_, unparseArgs) = getParseOption(arguments, UPDATE_SHORT_OPTION, UPDATE_LONG_OPTION)
    if (unparseArgs.size != 0) {
        eprintln("Error: unknown command '${unparseArgs[0]}' for cjpm update")
        return false
    }
    return true
}
