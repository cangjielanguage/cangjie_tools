// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*

let PUBLISH_USAGE: String = """
Publish module to cangjie repository.

Usage:
  cjpm publish [option]

Available options:
  -h, --help              help for publish
  -V, --verbose           enable verbose."""

let PUBLISH_LONG_OPTION: Array<String> = ["help", "verbose"]

class PublishCommand <: Handle {
    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..]
    }

    public override func printHelp(): Unit {
        println(PUBLISH_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        let (argMap, unparsedArgs) = getParseOption(this.arguments, "V", PUBLISH_LONG_OPTION)

        if (unparsedArgs.size != 0) {
            eprintln("Error: unknown command '${unparsedArgs[0]}' for cjpm publish")
            return false
        }

        let isVerbose = getBoolOption(argMap, "-V", "--verbose")
        return doPublish(isVerbose)
    }
}