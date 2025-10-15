// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*
import cjpm.config.*

let DEFAULT_INIT_TYPE: OutputType = Exe
let INIT_LONG_OPTION: Array<String> = ["name=", "path=", "type=", "workspace", "experimental"]
let INIT_USAGE: String = """
Initialize a new cangjie module.
This command creates "cjpm.toml" file, along with the necessary project directory "src".
If the '--workspace' option is specified, only the default configuration file for a workspace will be initialized.

Usage:
  cjpm init [option]

Available options:
  -h, --help                           help for init
  --workspace                          initialize a workspace's default configuration file
  --name <value>                       specify root package name, default as current directory name
  --path <value>                       specify path to create the module, default as current directory
  --type=<executable|static|dynamic>   define output type of current module
  --experimental                       initializes Cangjie Multiplatform project."""

// InitCommand represents the init command
class InitCommand <: Handle {
    private var outputType: OutputType

    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove init itself
        this.outputType = DEFAULT_INIT_TYPE
    }

    public override func printHelp(): Unit {
        println(INIT_USAGE)
        return
    }

    func handleUnparseArgs(unparseArgs: Array<String>): Bool {
        if (unparseArgs.isEmpty()) {
            return true
        }

        if (unparseArgs.contains("--name")) {
            eprintln("Error: the '--name' option needs a value, e.g.: cjModDemo")
            return false
        }

        if (unparseArgs.contains("--path")) {
            eprintln("Error: the '--path' option needs a value, e.g.: ./cjModDemo/")
            return false
        }

        eprintln("Error: unknown command '${unparseArgs[0]}' for cjpm init")
        return false
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "", INIT_LONG_OPTION)

        var (rootPkgName, nameFlag) = getArgValue(argMap, "--name")
        if (nameFlag && rootPkgName.isEmpty()) {
            eprintln("Error: the '--name' option needs a value, e.g.: cjModDemo")
            return false
        }
        var (modulePath, modulePathFlag) = getArgValue(argMap, "--path")
        if (modulePathFlag && modulePath.isEmpty()) {
            eprintln("Error: the '--path' option needs a value")
            return false
        }

        let (newType, typeFlag) = getArgValue(argMap, "--type")
        if (typeFlag) {
            let outputType = OutputType.fromString(newType)
            if (let Unknown(s) <- outputType) {
                eprintln("Error: unknown command '--type=${s}' for cjpm init")
                return false
            }
            this.outputType = outputType
        }

        let initExperimental: Bool = getBoolOption(argMap, "--experimental")
        let initWorkspace: Bool = getBoolOption(argMap, "--workspace")

        if (!handleUnparseArgs(unparseArgs)) {
            return false
        }

        return doInit(rootPkgName, modulePath, this.outputType, initExperimental, initWorkspace, typeFlag)
    }
}
