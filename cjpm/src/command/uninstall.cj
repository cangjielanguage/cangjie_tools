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

let UNINSTALL_USAGE: String = """
Uninstall a Cangjie binary.

Usage:
  cjpm uninstall name [option]

Arguments:
  name                      select binary to uninstall

Available option:
  -h, --help                help for uninstall
  --root <value>            specify path of uninstalled binary"""

class UninstallCommand <: Handle {
    private let UNINSTALL_LONG_OPTION: Array<String> = ["root="]
    private var uninstallList: ArrayList<String> = ArrayList<String>()
    private var root: String = Path(DEFAULT_CONFIG_LOCATION).join(".cjpm").toString()

    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..]
    }

    public override func printHelp(): Unit {
        println(UNINSTALL_USAGE)
        return
    }

    func handleUnparseArgs(unparseArgs: Array<String>): Bool {
        if (unparseArgs.size == 0) {
            eprintln("Error: need at least one binary to uninstall, but receive 0")
            return false
        }

        uninstallList.add(all: unparseArgs)

        return true
    }

    func handleRoot(argMap: HashMap<String, String>): Bool {
        let (rootPath, rootFlag) = getArgValue(argMap, "--root")
        if (rootFlag) {
            if (rootPath.isEmpty()) {
                eprintln("Error: the '--root' option cannot be empty")
                return false
            }
            root = rootPath
        }
        return true
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "", UNINSTALL_LONG_OPTION)

        if (!handleRoot(argMap)) {
            return false
        }

        if (!handleUnparseArgs(unparseArgs)) {
            return false
        }

        return doUninstall(root, uninstallList)
    }
}
