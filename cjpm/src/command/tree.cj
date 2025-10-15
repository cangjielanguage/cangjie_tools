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

let TREE_LONG_OPTION: Array<String> = ["help", "verbose", "depth=", "package=", "invert=", "target=", "no-tests",
    "skip-script"]
let TREE_VALUE_OPTIONS = ["-p", "--depth", "--package", "--invert", "--target"]
let TREE_USAGE: String = """
Display a tree visualization of a dependency graph

Usage:
  cjpm tree [option]

Available options:
  -h, --help                  help for tree
  -V, --verbose               enable verbose
  --depth <N>                 maximum display depth of the dependency tree
  -p, --package <value>       package to be used as the root of the tree
  --invert <value>            invert the tree direction and focus on the given package
  --target <value>            display dependencies for the given target platform
  --no-tests                  exclude the test-dependencies
  --skip-script               disable script 'build.cj'."""

class TreeCommand <: Handle {
    init(args: Array<String>) {
        this.cmdName = args[0]
        this.arguments = args[1..] // remove tree itself
    }

    public override func printHelp(): Unit {
        println(TREE_USAGE)
        return
    }

    func handleDepth(argMap: HashMap<String, String>, treeConfig: TreeConfig): Bool {
        let (depthStr, depthFlag) = getArgValue(argMap, "--depth")
        if (!depthFlag) {
            return true
        }

        var depth: Int32
        try {
            depth = Int32.parse(depthStr)
        } catch (e: Exception) {
            eprintln("Error: the 'depth' option requires a nonnegative integer argument, " +
                "its valid range is [0, 2147483647]")
            return false
        }

        if (depth < 0) {
            eprintln("Error: the 'depth' option requires a nonnegative integer argument, " +
                "its valid range is [0, 2147483647]")
            return false
        } else {
            treeConfig.depth = depth
            return true
        }
    }

    public override func handleCommand(): Bool {
        let (argMap, unparseArgs) = getParseOption(this.arguments, "p:Vh", TREE_LONG_OPTION)
        let treeConfig = TreeConfig()
        treeConfig.isVerbose = getBoolOption(argMap, "-V", "--verbose")
        treeConfig.noTest = getBoolOption(argMap, "--no-tests")
        treeConfig.defaultPkg = getArgValue(argMap, "-p", "--package")[0]
        treeConfig.invertPkg = getArgValue(argMap, "--invert")[0]

        if (!handleDepth(argMap, treeConfig)) {
            return false
        }

        var featureDeducer = FeatureDeducer(treeConfig.globalConfig.features,
            enabled: !treeConfig.globalConfig.noFeatureDeduce)

        match (handleCrossTarget(argMap)) {
            case None => return false
            case Some(true) => 
                match (Triple.fromString(crossCompileTarget)) {
                    case Ok(triple) => featureDeducer = featureDeducer.deduceTargetFeatures(triple)
                    case Err(errmesg) => eprintln(errmesg); return false
                }
            case Some(false) => featureDeducer = featureDeducer.deduceTargetFeatures(state.cjcInfo.target)
        }

        if (unparseArgs.size != 0) {
            for (val in TREE_VALUE_OPTIONS) {
                if (unparseArgs.contains(val)) {
                    eprintln("Error: the '${val}' option needs a value")
                    return false
                }
            }
            eprintln("Error: unknown command '${unparseArgs[0]}' for cjpm tree")
            return false
        }

        if (let Some(features) <- handleEnableFeaturesOption(argMap)) {
            featureDeducer = featureDeducer.addFeature(features)
        }
        treeConfig.globalConfig.features = featureDeducer.deduceDependantFeatures().collect()

        return doTree(treeConfig)
    }
}
