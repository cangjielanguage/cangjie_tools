// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.command

import std.collection.*
import std.convert.*
import cjpm.implement.*
import cjpm.config.*

public let TEST_LONG_OPTION: Array<String> = ["filter=", "include-tags=", "exclude-tags=", "verbose", "no-run",
    "skip-build", "target=", "target-dir=", "enable-features=", "no-feature-deduce", "coverage", "jobs=", "no-color",
    "random-seed=", "timeout-each=", "parallel=", "member=", "report-format=", "report-path=", "skip-script",
    "no-capture-output", "show-all-output", "module=", "incremental", "dry-run", "no-progress", "progress-brief",
    "progress-entries-limit=", "show-tags"]
let TEST_USAGE: String = """
Build and run unittest for all packages in current module.
If you specify some package paths, it will only test those specified packages.

Usage:
  cjpm test [option] [path1 path2 ...]

Available options:
  -h, --help                              help for test
  -j, --jobs <N>                          the number of jobs to spawn in parallel during the test process
  -V, --verbose                           enable verbose
  -g                                      enable compile debug version tests
  -i, --incremental                       enable incremental compilation
  --no-run                                compile, but don't run tests
  --skip-build                            skip compile, only run tests
  --coverage                              enable coverage
  --cfg                                   enable the customized option 'cfg'
  --module <value|"value1 value2">        specify modules to test, default as current module
  -m, --member <value>                    specify a member module of the workspace
  --target <value>                        unittest for the given target platform
  --target-dir <value>                    specify target directory
  --dry-run                               print tests without execution
  --enable-features <value>               explicitly specify comma-separated list of features to be enabled
  --no-feature-deduce                     disables auto-enabling of features, deduced from other options, machine properties, etc.
  --filter <value>                        enable filter test
  --include-tags <value>                  run tests with specified tags
  --exclude-tags <value>                  run tests without specified tags
  --no-color                              enable colorless result output
  --random-seed <N>                       enable random seed
  --timeout-each=%d[millis|s|m|h]         specify default timeout for test cases
  --parallel <N>                          specify number of workers running tests
  --show-tags                             print tags in text report
  --show-all-output                       enable test output printing for all test cases including passed ones
  --no-capture-output                     disable test output capturing, output will be printed during test execution immediately
  --report-path <value>                   specify path to directory of report
  --report-format <value>                 specify format of report
  --skip-script                           disable script 'build.cj'
  --no-progress                           disable progress report
  --progress-brief                        display brief progress report instead of detailed one
  --progress-entries-limit                limit the entries amount displayed in the progress report."""

// TestCommand represents the test command
class TestCommand <: Handle {
    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove subcommand itself
    }

    public override func printHelp(): Unit {
        println(TEST_USAGE)
        return
    }

    static func handleUnparseArgs(unparseArgs: Array<String>): Bool {
        if (unparseArgs.isEmpty()) {
            return true
        }

        if (unparseArgs.contains("-j") || unparseArgs.contains("--jobs")) {
            eprintln("Error: the '-j/--jobs' option requires a positive integer argument, " +
                "its valid range is (0, 2147483647]")
            return false
        }

        let options = ["--filter", "--include-tags", "--exclude-tags", "--target", "--target-dir", "--enable-features",
            "--random-seed", "--timeout-each", "--parallel", "--report-path", "--report-format", "--no-progress",
            "--progress-brief", "--progress-entries-limit"]
        for (val in options) {
            if (unparseArgs.contains(val)) {
                eprintln("Error: the '${val}' option needs a value")
                return false
            }
        }

        return true
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "gVj:m:i", TEST_LONG_OPTION)

        let testConfig = processTestCliOptions(argMap, unparseArgs) ?? return false

        doTest(unparseArgs, testConfig)
    }

    static func processTestCliOptions(argMap: HashMap<String, String>, unparseArgs: Array<String>): ?TestConfig {
        let testConfig = TestConfig()
        testConfig.isCov = getBoolOption(argMap, "--coverage")
        testConfig.noRun = getBoolOption(argMap, "--no-run")
        testConfig.showTags = getBoolOption(argMap, "--show-tags")
        testConfig.noCaptureOutput = getBoolOption(argMap, "--no-capture-output")
        testConfig.showAllOutput = getBoolOption(argMap, "--show-all-output")
        testConfig.skipBuild = getBoolOption(argMap, "--skip-build")
        testConfig.isVerbose = getBoolOption(argMap, "-V", "--verbose")
        testConfig.noColor = getBoolOption(argMap, "--no-color")
        testConfig.isDebug = getBoolOption(argMap, "-g")
        testConfig.isIncremental = getBoolOption(argMap, "-i", "--incremental")
        testConfig.dryRun = getBoolOption(argMap, "--dry-run")
        testConfig.globalConfig.isSkipScript = getBoolOption(argMap, "--skip-script")
        testConfig.globalConfig.noFeatureDeduce = getBoolOption(argMap, "--no-feature-deduce")
        if (!state.isExperimental && testConfig.globalConfig.noFeatureDeduce) {
            eprintln("Error: `--no-feature-deduce` is part of experimental Cangjie Multiplatform feature, " +
                "please enable `experimental = true` in cjpm.toml of this project")
            return None
        }

        var featureDeducer = FeatureDeducer(handleEnableFeaturesOption(argMap) ?? return None,
            enabled: !testConfig.globalConfig.noFeatureDeduce)

        let (testModule, hasTestModule) = getArgValue(argMap, "--module")
        if (hasTestModule) {
            if (testModule.isEmpty()) {
                eprintln("Error: the '--module' option can't be empty")
                return None
            }
            testConfig.testModule = HashSet<String>(testModule.split(" "))
        }
        let (timeout, hasTimeoutEach) = getArgValue(argMap, "--timeout-each")
        if (hasTimeoutEach) {
            testConfig.timeoutEach = timeout
        }
        let (nWorkers, hasParallel) = getArgValue(argMap, "--parallel")
        if (hasParallel) {
            testConfig.nWorkers = nWorkers
        }
        let (seed, hasRandomSeed) = getArgValue(argMap, "--random-seed")
        if (hasRandomSeed) {
            testConfig.randomSeed = seed
        }

        testConfig.filters = handleFilterOption(argMap)
        testConfig.includeTags = handleTagsOption(argMap, isInclude: true)
        testConfig.excludeTags = handleTagsOption(argMap, isInclude: false)

        testConfig.targetDir = handleTargetDirOption(argMap) ?? return None

        let (reportFormat, hasReportFormat) = getArgValue(argMap, "--report-format")
        let (reportPath, hasReportPath) = getArgValue(argMap, "--report-path")

        if (hasReportFormat) {
            testConfig.reportFormat = reportFormat
        }
        if (hasReportPath) {
            testConfig.reportPath = reportPath
        }

        testConfig.isCrossCompile = handleCrossTarget(argMap) ?? return None
        testConfig.memberModule = handleMemberOption(argMap) ?? return None
        if (!testConfig.memberModule.isEmpty() && !testConfig.testModule.isEmpty()) {
            eprintln("Error: -m/--member option option conflicts with module test by '--module'")
            return None
        }

        if (!handleParallelSize(argMap)) {
            return None
        }
        testConfig.isSpecifiedConcurrency = getBoolOption(argMap, "-j", "--jobs")

        testConfig.isProgressReportDisabled = getBoolOption(argMap, "--no-progress")
        testConfig.isProgressReportBrief = getBoolOption(argMap, "--progress-brief")

        let (progressReportEntriesLimit, hasProgressReportEntriesLimit) = getArgValue(argMap, "--progress-entries-limit"
        )
        if (hasProgressReportEntriesLimit) {
            try {
                let limit = Int64.parse(progressReportEntriesLimit)
                testConfig.progressReportEntriesLimit = limit
            } catch (e: Exception) {
                eprintln("Error: the '--progress-entries-limit' option requires a positive integer argument " +
                    "or 0 if there is no limit (default)")
                return None
            }
        }

        if (!handleUnparseArgs(unparseArgs)) {
            return None
        }

        testConfig.globalConfig.features = featureDeducer
            .deduceTargetFeatures(state.cjcInfo.target)
            .deduceDependantFeatures()
            .collect()

        testConfig
    }
}
