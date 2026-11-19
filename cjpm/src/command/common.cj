// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import std.argopt.*
import std.collection.*
import std.regex.*
import cjpm.util.*
import cjpm.config.*

func getArgValue(argMap: HashMap<String, String>, longOpt: String, shortOpt: String): (String, Bool) {
    if (argMap.contains(longOpt)) {
        return (argMap[longOpt], true)
    }
    if (argMap.contains(shortOpt)) {
        return (argMap[shortOpt], true)
    }
    return ("", false)
}

func getArgValue(argMap: HashMap<String, String>, opt: String): (String, Bool) {
    if (argMap.contains(opt)) {
        return (argMap[opt], true)
    }
    return ("", false)
}

func getBoolOption(argMap: HashMap<String, String>, arg: String): Bool {
    return argMap.contains(arg)
}

func getBoolOption(argMap: HashMap<String, String>, shortArg: String, longArg: String): Bool {
    return (argMap.contains(shortArg) || argMap.contains(longArg))
}

func isCompatVersionValue(value: String): Bool {
    Regex("^([0-9]+)[.]([0-9]+)$").find(value).isSome()
}

/*
 * Probe parse without the compatibility option registered: every standalone '--compat-version'
 * token lands in the probe's unparseArgs, while values of other options are still consumed
 * with proper option boundaries — an argument meant for the application is never mistaken
 * for a cjpm option. Returns all specified values in order; a missing value is reported by
 * the caller from the real parse's unparseArgs.
 */
func collectCompatVersionValues(sysArgs: Array<String>, shortArg: String, longArgList: Array<String>,
    option: String): ArrayList<String> {
    let probeLongs = ArrayList<String>()
    for (item in longArgList) {
        if (item != option[2..] + "=") {
            probeLongs.add(item)
        }
    }
    var ao = ArgOpt(sysArgs, shortArg, probeLongs.toArray())
    let unparseArgs = ao.getUnparseArgs()
    let values = ArrayList<String>()
    var index = 0
    while (index < unparseArgs.size) {
        let token = unparseArgs[index]
        if (token == option) {
            if (index + 1 < unparseArgs.size && !unparseArgs[index + 1].startsWith("-")) {
                values.add(unparseArgs[index + 1])
                index += 2
                continue
            }
            index += 1
            continue
        }
        if (token.startsWith(option + "=")) {
            values.add(token[(option + "=").size..])
        }
        index += 1
    }
    return values
}

/*
 * Confirm after `getParseOption` that '--compat-version' is not specified with conflicting
 * values, validate every specified value and record the compatibility target for the build.
 *
 * Conflicting duplicates cannot be read from `argMap` (std.argopt silently keeps the last
 * value of a duplicated option), so the probe parse collects all occurrences with proper
 * option boundaries — an argument meant for the application is never mistaken for a cjpm
 * option. Only the two-segment `major.minor` format is checked: the supported version
 * whitelist is owned by cjc, cjpm forwards the value and lets the compiler report
 * unsupported versions. A missing value is reported by the caller from the real parse's
 * unparseArgs.
 */
public func handleCompatVersionOption(sysArgs: Array<String>, shortArg: String, longArgList: Array<String>,
    argMap: HashMap<String, String>): Bool {
    let option = "--compat-version"
    let values = collectCompatVersionValues(sysArgs, shortArg, longArgList, option)
    if (values.size == 0) {
        // The probe attributed no standalone occurrence: either the option was not specified,
        // or the real parse consumed a token that the probe could not attribute (e.g. a
        // missing value followed by another option). Validate what the real parse extracted.
        if (let (value, true) <- getArgValue(argMap, option)) {
            if (!isCompatVersionValue(value)) {
                eprintln("Error: invalid value '${value}' for '${option}', the example is '1.3'")
                return false
            }
            compatVersion = value
        }
        return true
    }

    var version = ""
    for (value in values) {
        if (!isCompatVersionValue(value)) {
            eprintln("Error: invalid value '${value}' for '${option}', the example is '1.3'")
            return false
        }
        if (version != "" && version != value) {
            eprintln("Error: the '${option}' option is specified with different values " +
                "'${version}' and '${value}'")
            return false
        }
        version = value
    }
    if (version != "") {
        compatVersion = version
    }
    return true
}

public func getParseOption(sysArgs: Array<String>, shortArg: String, longArgList: Array<String>):
    (HashMap<String, String>, Array<String>) {
    var ao = ArgOpt(sysArgs, shortArg, longArgList)
    let argMap: HashMap<String, String> = ao.getArgumentsMap()
    let unparseArgs = ao.getUnparseArgs()
    return (argMap, unparseArgs)
}

public func hasHelpCommand(cmdHandler: Handle): Bool {
    if (cmdHandler.isHelp()) {
        cmdHandler.printHelp()
        if (cmdHandler.arguments.size > 1) {
            eprintln("Warning: cjpm doesn't do job in show usage mode")
        }
        return true
    }
    return false
}

func handleTargetDirOption(argMap: HashMap<String, String>): Option<String> {
    let (newDir, dirFlag) = getArgValue(argMap, "--target-dir")
    if (dirFlag) {
        if (newDir.isEmpty()) {
            eprintln("Error: the target directory can't be empty")
            return None
        }
        if (!safeCheck(newDir, "target-dir")) {
            return None
        }
    }
    return Some(newDir)
}

func handleEnableFeaturesOption(argMap: HashMap<String, String>): Result<Array<Feature>, Array<CJMPError>> {
    let flag = "enable-features"
    let (featureString, featuresFlag) = getArgValue(argMap, "--${flag}")
    if (featuresFlag && !safeCheck(featureString, flag)) {
        return Err([])
    }
    if (featureString.isEmpty()) {
        return Ok([])
    }

    featureString.trimAscii().split(",", removeEmpty: true).allResult { it =>
        QName.fromString(it)
            .mapErr { e => ErrorWrapper(None, e) }
            .flatMap { it => Feature.create(it) }.mapErr { e => [ e ] } }
}

func handleFilterOption(argMap: HashMap<String, String>): Option<String> {
    let flag = "filter"
    let (filterOptions, filterFlag) = getArgValue(argMap, "--${flag}")
    if (filterFlag && !safeCheck(filterOptions, flag)) {
        return None
    }
    if (filterOptions.isEmpty()) {
        return None
    }
    return Some(filterOptions)
}

func handleTagsOption(argMap: HashMap<String, String>, isInclude!: Bool = true): Option<String> {
    let flag = if (isInclude) {
        "include-tags"
    } else {
        "exclude-tags"
    }
    let (tagsOptions, tagsFlag) = getArgValue(argMap, "--${flag}")
    if (tagsFlag && !safeCheck(tagsOptions, flag)) {
        return None
    }
    if (tagsOptions.isEmpty()) {
        return None
    }
    return Some(tagsOptions)
}

public func handleMemberOption(argMap: HashMap<String, String>): Option<String> {
    let (member, memFlag) = getArgValue(argMap, "--member", "-m")
    if (memFlag) {
        if (member.isEmpty()) {
            eprintln("Error: the member value can't be empty")
            return None
        }
    }
    return Some(member)
}

func handleCrossTarget(argMap: HashMap<String, String>): Option<Bool> {
    let (target, crossCompileFlag) = getArgValue(argMap, "--target")
    if (crossCompileFlag) {
        let (hasTarget, validTarget) = crossCompileCheck(target)
        if (!hasTarget) {
            return None
        }
        return Some(validTarget)
    }
    return Some(crossCompileFlag)
}

func crossCompileCheck(target: String): (Bool, Bool) {
    if (target.size == 0) {
        eprintln("Error: the cross-compile target can't be empty")
        return (false, false)
    }

    let invalidTargetMessage =
        "Error: the cross-compile target '${target}' is wrong, the example is 'aarch64-hm-gnu'"
    if (target == "." || target.contains("..")) {
        eprintln(invalidTargetMessage)
        return (false, false)
    }
    for (segment in target.split("-")) {
        if (segment == ".") {
            eprintln(invalidTargetMessage)
            return (false, false)
        }
    }

    let REGEXP_CROSS_COMPILE = "^([-_a-zA-Z0-9.]*)$"
    var matched = Regex(REGEXP_CROSS_COMPILE)
    match (matched.find(target)) {
        case Some(_) =>
            if (target == targetConfigName) {
                eprintln("Warning: the current platform is '${target}', the '--target' option will be ignored")
                return (true, false)
            }
            if (normalizeTarget(target) == normalizeTarget(targetConfigName)) {
                eprintln("Warning: target '${target}' is equivalent to the local target " +
                    "'${targetConfigName}', cjpm will perform native-compilation instead of cross-compilation")
                return (true, false)
            }
            crossCompileTarget = target
            return (true, true)
        case None => eprintln(invalidTargetMessage)
    }

    return (false, false)
}

public func lackConfigInfo(): Bool {
    !FINAL_CJPM_CONFIG_INFO.isValid
}

public func findCjc(): Unit {
    targetConfigName = state.cjcInfo.target.toString()
}
