// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import std.collection.*
import std.fs.*
import cjpm.implement.*
import cjpm.config.*

class ScriptArgs {
    ScriptArgs(let isDebug: Bool, let isVerbose: Bool, let isCross: Bool, let isSkip: Bool) {}
}

var scriptConfig: ScriptConfig = ScriptConfig()

func getArgList(cmdName: String, args: Array<String>): (Bool, ScriptArgs) {
    var shortOption: String = ""
    var longOption: ArrayList<String> = ArrayList<String>(["skip-script"])

    if (DEBUG_CMD.contains(cmdName)) {
        shortOption = shortOption + "g"
    }

    if (VERBOSE_CMD.contains(cmdName)) {
        shortOption = shortOption + "V"
        longOption.add("verbose")
    }

    if (CROSS_CMD.contains(cmdName)) {
        longOption.add("target=")
    }

    let (argMap, unparseArgs) = getParseOption(args, shortOption, longOption.toArray())
    if (unparseArgs.contains("--target")) {
        eprintln("Error: the '--target' option needs a value")
        return (false, ScriptArgs(false, false, false, false))
    }

    let skip: Bool = getBoolOption(argMap, "--skip-script")
    if (skip) {
        return (true, ScriptArgs(false, false, false, true))
    }

    var cross: Bool = false
    if (let Some(val) <- handleCrossTarget(argMap)) {
        cross = val
    } else {
        return (false, ScriptArgs(false, false, false, false))
    }

    let debug: Bool = getBoolOption(argMap, "-g")
    let verbose: Bool = getBoolOption(argMap, "-V", "--verbose")

    return (true, ScriptArgs(debug, verbose, cross, skip))
}

public func runScriptPreCommand(cmdName: String, args: Array<String>): Bool {
    if (!BUILD_SCRIPT_CMD_PRE.contains(cmdName)) {
        return true
    }
    let (flag, scriptArgs) = getArgList(cmdName, args)
    if (!flag) {
        return false
    }
    if (scriptArgs.isSkip) {
        return true
    }
    scriptConfig = makeScriptConfig(DIR_CURRENT, scriptArgs.isDebug, scriptArgs.isVerbose, scriptArgs.isCross) ?? return false
    return runScript(scriptConfig, Pre, cmdName)
}

public func runScriptPostCommand(cmdName: String): Bool {
    if (!BUILD_SCRIPT_CMD_POST.contains(cmdName)) {
        return true
    }
    return runScript(scriptConfig, Post, cmdName)
}

public func postCommandScript(): Bool {
    try {
        // return build.cj.bak to build.cj
        for (bakScript in RELOCATED_SCRIPT_MAP) {
            rename(bakScript.removeSuffix("build.cj") + "build.cj.bak", to: bakScript, overwrite: true)
        }
        return true
    } catch (e: Exception) {
        eprintln("${e.message}\nError: failed to restore build.cj, please check permission")
        return false
    }
}
