// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*

let CHECK_SHORT_OPTION: String = "m:"

class CheckCommand <: Handle {
    private let CHECK_USAGE: String = """
Check the dependencies of the current project.

Usage:
  cjpm check [option]

Available option:
  -m, --member <value>      specify a member module of the workspace
  -h, --help                help for check
  --no-tests                exclude the test-dependencies
  --compat-version <value>  check dependencies according to the specified compatibility version
  --skip-script             disable script 'build.cj'."""

    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove check itself
        this.shortOptionSpec = CHECK_SHORT_OPTION
        this.longOptionSpecs = ["member=", "no-tests", "skip-script", "compat-version="]
    }

    public override func printHelp(): Unit {
        println(CHECK_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        let (parseFlag, memberModule, testCheck) = parseOptions(this.arguments, this.longOptionSpecs)
        if (!parseFlag) {
            return false
        }
        return doCheck(memberModule, testCheck)
    }
}

func parseOptions(arguments: Array<String>, longArgList: Array<String>): (Bool, String, Bool) {
    var memberModule = ""
    var testCheck = true
    let (argMap, unparseArgs) = getParseOption(arguments, CHECK_SHORT_OPTION, longArgList)
    if (!handleCompatVersionOption(arguments, CHECK_SHORT_OPTION, longArgList, argMap)) {
        return (false, memberModule, testCheck)
    }
    if (let Some(val) <- handleMemberOption(argMap)) {
        memberModule = val
    } else {
        return (false, memberModule, testCheck)
    }
    let options = ["-m", "--member", "--compat-version"]
    for (val in options) {
        if (unparseArgs.contains(val)) {
            eprintln("Error: the '${val}' option needs a value")
            return (false, memberModule, testCheck)
        }
    }
    if (unparseArgs.size != 0) {
        eprintln("Error: unknown command '${unparseArgs[0]}' for cjpm check")
        return (false, memberModule, testCheck)
    }
    testCheck = !getBoolOption(argMap, "--no-tests")
    return (true, memberModule, testCheck)
}
