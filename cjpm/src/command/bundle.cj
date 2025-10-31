// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.config.*
import cjpm.implement.*

let BUNDLE_USAGE: String = """
Assemble local cjpm module into a distributable tarball

Usage:
  cjpm bundle [option]

Available options:
  -h, --help              help for bundle
  -V, --verbose           enable verbose
  --skip-test             make tarball without running cjpm test
  --skip-lint             make tarball without running cjlint."""

let BUNDLE_LONG_OPTION: Array<String> = ["help", "verbose", "skip-test", "skip-lint"]

class BundleCommand <: Handle {
    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..]
    }

    public override func printHelp(): Unit {
        println(BUNDLE_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        let (argMap, unparsedArgs) = getParseOption(this.arguments, "V", BUNDLE_LONG_OPTION)

        if (unparsedArgs.size != 0) {
            eprintln("Error: unknown command '${unparsedArgs[0]}' for cjpm bundle")
            return false
        }

        let bundleConfig = BundleConfig()
        bundleConfig.isVerbose = getBoolOption(argMap, "-V", "--verbose")
        bundleConfig.isTest = !getBoolOption(argMap, "--skip-test")
        bundleConfig.isLint = !getBoolOption(argMap, "--skip-lint")
        return doBundle(bundleConfig)
    }
}