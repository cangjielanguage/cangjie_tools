// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjcov.util

import std.posix.*
import std.process.*
import std.fs.*
import std.collection.*
import std.argopt.*

func getArgValue(argMap: HashMap<String, String>, longOpt: String, shortOpt: String): String {
    if (argMap.contains(longOpt)) {
        return argMap[longOpt]
    }

    if (argMap.contains(shortOpt)) {
        return argMap[shortOpt]
    }

    return ""
}

func getOptionStoreTrue(argMap: HashMap<String, String>, arg: String) {
    return argMap.contains(arg)
}

func getBoolOption(argMap: HashMap<String, String>, shortArg: String, longArg: String): Bool {
    return (argMap.contains(shortArg) || argMap.contains(longArg))
}

public class Command {
    public var version: Bool = false
    public var help: Bool = false
    public var keep: Bool
    public var htmldetail: Bool
    public var branches: Bool
    public var verbose: Bool
    public var xml: Bool
    public var json: Bool
    public var output: String
    public var rootArr: ArrayList<String>
    public var sourceArr: ArrayList<String>
    public var excludeArr: ArrayList<String>
    public var includeArr: ArrayList<String>

    public init(argMap: HashMap<String, String>) {
        version = getBoolOption(argMap, "-v", "--version")
        help = getBoolOption(argMap, "-h", "--help")
        keep = getBoolOption(argMap, "-k", "--keep")
        branches = getBoolOption(argMap, "-b", "--branches")
        xml = getBoolOption(argMap, "-x", "--xml")
        json = getBoolOption(argMap, "-j", "--json")

        htmldetail = getOptionStoreTrue(argMap, "--html-details")
        verbose = getOptionStoreTrue(argMap, "--verbose")

        output = getArgValue(argMap, "--output", "-o")
        var rootArg = getArgValue(argMap, "--root", "-r")
        rootArr = ArrayList<String>(rootArg.split(" ", removeEmpty: true))
        var sourceArg = getArgValue(argMap, "--source", "-s")
        sourceArr = ArrayList<String>(sourceArg.split(" ", removeEmpty: true))
        var excludeArg = getArgValue(argMap, "--exclude", "-e")
        excludeArr = ArrayList<String>(excludeArg.split(" ", removeEmpty: true))
        var includeArg = getArgValue(argMap, "--include", "-i")
        includeArr = ArrayList<String>(includeArg.split(" ", removeEmpty: true))
    }
}

public func getParseOption(sysArgs: Array<String>): HashMap<String, String> {
    var longArgList = [
        "root=",
        "output=",
        "html-details",
        "branches",
        "html",
        "keep",
        "version",
        "help",
        "verbose",
        "keep",
        "xml",
        "json",
        "source=",
        "exclude=",
        "include="
    ]
    var shortArg = "r:o:vhbkxjs:e:i:"
    var ao = ArgOpt(sysArgs, shortArg, longArgList)

    return ao.getArgumentsMap()
}

// Execute the command and store the output.
public func subprocessCallWithOutput(exeTool: String, arguments: ArrayList<String>, workingDirectory!: ?Path = None): (Bool, 
    String, String) {
    var returnCode: Int64
    var outInfo = Array<UInt8>()
    var errInfo = Array<UInt8>()
    try {
        (returnCode, outInfo, errInfo) = Process.runOutput(exeTool, arguments.toArray(),
            workingDirectory: workingDirectory, stdOut: ProcessRedirect.Pipe, stdErr: ProcessRedirect.Pipe)
        return (returnCode == 0, String.fromUtf8(outInfo), String.fromUtf8(errInfo))
    } catch (e: Exception) {
        eprintln(e.message)
        return (false, String.fromUtf8(outInfo), String.fromUtf8(errInfo))
    }
}

public func existSpecificSymbol(path: String): Bool {
    let specialSymobls = ["|", ";", "&", ">", "<", "`", "!"]
    for (s in specialSymobls) {
        if (path.contains(s)) {
            return true
        }
    }
    return false;
}
