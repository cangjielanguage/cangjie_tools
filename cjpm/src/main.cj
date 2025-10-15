// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm

import std.env.*
import cjpm.command.*
import cjpm.util.CliFailError

func getCommandArgs(): Array<String> {
    return getCommandLine()[1..]
}

main(): Int64 {
    let args = getCommandArgs()

    let factory = CommandFactory()
    let cmdHandler = factory.createCommandHandler(args)
    try {
        if (hasHelpCommand(cmdHandler)) {
            return 0
        }

        if (lackCjc()) {
            return 1
        }

        if (!runScriptPreCommand(cmdHandler.cmdName, cmdHandler.arguments)) {
            cmdHandler.printError()
            return 1
        }

        if (!cmdHandler.handleCommand()) {
            cmdHandler.printError()
            return 1
        }

        if (!runScriptPostCommand(cmdHandler.cmdName)) {
            cmdHandler.printError()
            return 1
        }

        if (!postCommandScript()) {
            cmdHandler.printError()
            return 1
        }

        cmdHandler.printPass()
    } catch (e: CliFailError) {
        eprintln(e.message)
        cmdHandler.printError()
        return 1
    }

    return 0
}
