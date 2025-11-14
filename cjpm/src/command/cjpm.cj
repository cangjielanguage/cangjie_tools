// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import std.collection.*
import cjpm.implement.*

public let CJPM_USAGE: String = """
Cangjie Package Manager

Usage:
  cjpm [subcommand] [option]

Available subcommands:
  init             Init a new cangjie module
  check            Check the dependencies
  update           Update cjpm.lock
  tree             Display the package dependencies in the source code
  build            Compile the current module
  run              Compile and run an executable product
  test             Unittest a local package or module
  bench            Run benchmarks in a local package or module
  clean            Clean up the target directory
  install          Install a cangjie binary
  uninstall        Uninstall a cangjie binary

Available options:
  -h, --help       help for cjpm
  -v, --version    version for cjpm

Use "cjpm [subcommand] --help" for more information about a command."""

public open class Handle {
    public var cmdName: String = ""
    public var arguments: Array<String> = Array<String>()

    public open func handleCommand(): Bool {
        return false
    }

    public open func isHelp(): Bool {
        if (this.arguments.isEmpty()) {
            return false
        }
        return this.arguments.contains("-h") || this.arguments.contains("--help")
    }

    public open func printHelp(): Unit {
        return
    }

    public open func printError(): Unit {
        eprintln("Error: cjpm ${cmdName} failed")
    }

    public open func printPass(): Unit {
        println("cjpm ${cmdName} success")
    }
}

// CpmCommand represents the base command when called without any subcommands
class CpmCommand <: Handle {
    private let CJPM_VERSION: String = "Cangjie Package Manager: 1.0.5"

    init(args: Array<String>) {
        this.arguments = args
    }

    public override func isHelp(): Bool {
        if (this.arguments.isEmpty()) {
            return false
        }
        return this.arguments[0] == "-h" || this.arguments[0] == "--help"
    }

    public override func printHelp(): Unit {
        println(CJPM_USAGE)
        return
    }

    public override func printError(): Unit {
        eprintln("Error: cjpm failed")
    }

    public override func printPass(): Unit {
        return
    }

    func getVersion(): String {
        return CJPM_VERSION
    }

    public override func handleCommand(): Bool {
        if (this.arguments.size == 0) {
            println(CJPM_USAGE)
            return true
        }

        let isVersion: Bool = (this.arguments[0] == "-v") || (this.arguments[0] == "--version")

        if (isVersion) {
            println(getVersion())
            if (this.arguments.size > 1) {
                eprintln("Warning: cjpm doesn't do job in show usage mode")
            }
            return true
        }

        return checkExpandedCmd(ArrayList<String>(this.arguments))
    }
}
