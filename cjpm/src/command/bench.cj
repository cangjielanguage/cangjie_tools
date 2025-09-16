// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import cjpm.implement.*

let BENCH_LONG_OPTION: Array<String> = ["filter=", "include-tags=", "exclude-tags=", "verbose", "no-run", "skip-build",
    "target=", "target-dir=", "jobs=", "no-color", "random-seed=", "member=", "report-format=", "report-path=",
    "baseline-path=", "skip-script", "module=", "incremental", "dry-run", "show-tags"]
let BENCH_USAGE: String = """
Build and run benchmarks for all packages.
If you specify some package paths, it will only execute benchmarks in those specified packages.

Usage:
  cjpm bench [option] [path1 path2 …]

Available options:
  -h, --help                              help for test
  -j, --jobs <N>                          the number of jobs to spawn in parallel during the test process
  -V, --verbose                           enable verbose
  -g                                      enable compile debug version tests
  -i, --incremental                       enable incremental compilation
  --no-run                                compile, but don't run tests
  --skip-build                            skip compile, only run tests
  --cfg                                   enable the customized option 'cfg'
  --module <value|"value1 value2">        specify modules to test, default as current module
  -m, --member <value>                    specify a member module of the workspace
  --target <value>                        unittest for the given target platform
  --target-dir <value>                    specify target directory
  --dry-run                               print benchmarks without execution
  --filter <value>                        enable filter test
  --include-tags <value>                  run tests with specified tags
  --exclude-tags <value>                  run tests without specified tags
  --no-color                              enable colorless result output
  --show-tags                             print tags in text report
  --random-seed <N>                       enable random seed
  --report-path <value>                   specify path to directory of report
  --report-format <value>                 specify format of report
  --baseline-path <value>                 specify path to directory of report to compare the results with
  --skip-script                           disable script 'build.cj'."""

// BenchCommand represents the bench command
class BenchCommand <: Handle {
    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove subcommand itself
    }

    public override func printHelp(): Unit {
        println(BENCH_USAGE)
        return
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "gVj:m:i", BENCH_LONG_OPTION)

        let testConfig = TestCommand.processTestCliOptions(argMap, unparseArgs) ?? return false

        for (unparsed in unparseArgs) {
            for (testOption in TEST_LONG_OPTION) {
                if (unparsed.removePrefix("--").contains(testOption)) {
                    eprintln("Error: the '${unparsed}' option is not supported with bench subcommand")
                    return false
                }
            }
        }

        testConfig.isBench = true
        testConfig.noCaptureOutput = true
        testConfig.nWorkers = None

        if (let (baselinePath, true) <- getArgValue(argMap, "--baseline-path")) {
            testConfig.baselinePath = baselinePath
        }

        doTest(unparseArgs, testConfig)
    }
}
