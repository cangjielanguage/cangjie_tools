// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

public func doTree(treeConfig: TreeConfig): Bool {
    if (!treeConfig.defaultPkg.isEmpty() && !treeConfig.invertPkg.isEmpty()) {
        eprintln("Error: the '-p/--package' option and '--invert' option " +
            "are not allowed to be specified at the same time")
        return false
    }

    let tomlInfo: TomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    if (!checkForWorkspace(DIR_CURRENT, tomlInfo, "", isTest: !treeConfig.noTest)) {
        return false
    }

    var configFileDir: String
    if (!COMMON_INFO.inWorkspace) {
        if (!replaceEnvforTargetDir(tomlInfo, Path(DIR_CURRENT).join(CONFIG_FILE_NAME).toString())) {
            return false
        }
        configFileDir = tomlInfo.pkg.targetDir
    } else {
        configFileDir = COMMON_INFO.targetDir
    }

    treeConfig.globalConfig.targetDir = getOriginTargetDirectory(treeConfig.globalConfig.targetDir, DIR_CURRENT, "",
        configFileDir, false) ?? return false

    return startTree(treeConfig)
}

private func printMultiplatformSourceLocations(packageDepMap: HashMap<String, ResolveItem>, features: Iterable<Feature>): Unit {
    var first = true
    for ((name, v) in packageDepMap) {
        if (v.isMultiplatform) {
            if (first) {
                print('\n');
                first = false
            }
            println(SourceLocationsPrinter(v.packagePath, features, project: name))
        }
    }
}

func startTree(treeConfig: TreeConfig): Bool {
    treeConfig.globalConfig.isTest = !treeConfig.noTest
    let (dataModel, flag, _) = resolve(treeConfig.globalConfig)
    if (!flag) {
        return false
    }
    var packageOrder = dataModel.readyPkgs
    packageOrder.reverse()

    for ((fullName, v) in dataModel.packageDepMap) {
        treeConfig.deps[fullName] = TreeItem(v.packagePath, v.version, HashSet<String>(v.requires))
    }

    if (!treeConfig.defaultPkg.isEmpty()) {
        if (!packageOrder.contains(treeConfig.defaultPkg)) {
            eprintln("Error: the package '${treeConfig.defaultPkg}' does not exist")
            return false
        }
        printDependencyTree(treeConfig, treeConfig.defaultPkg, 0)
        printMultiplatformSourceLocations(dataModel.packageDepMap, treeConfig.globalConfig.features)
        return true
    }

    if (!treeConfig.invertPkg.isEmpty()) {
        if (!packageOrder.contains(treeConfig.invertPkg)) {
            eprintln("Error: the package '${treeConfig.invertPkg}' does not exist")
            return false
        }
        printInvertTree(treeConfig, treeConfig.invertPkg, 0)
        printMultiplatformSourceLocations(dataModel.packageDepMap, treeConfig.globalConfig.features)
        return true
    }

    for (name in packageOrder) {
        if (treeConfig.treePkgs.contains(name) && treeConfig.depth < 0) {
            continue
        }
        printDependencyTree(treeConfig, name, 0)
    }

    printMultiplatformSourceLocations(dataModel.packageDepMap, treeConfig.globalConfig.features)
    return true
}

func printDependencyTree(treeConfig: TreeConfig, name: String, level: Int32): Unit {
    if (!treeConfig.deps.contains(name)) {
        return
    }
    treeConfig.treePkgs.add(name)

    printTreeStr(treeConfig, name, level)

    if (treeConfig.depth >= 0 && treeConfig.depth == level) {
        return
    }

    for (child in treeConfig.deps[name].requires) {
        printDependencyTree(treeConfig, child, level + 1)
    }
    return
}

func printInvertTree(treeConfig: TreeConfig, name: String, level: Int32): Unit {
    printTreeStr(treeConfig, name, level)

    if (treeConfig.depth >= 0 && treeConfig.depth == level) {
        return
    }

    for ((fullName, dependencies) in treeConfig.deps) {
        for (dep in dependencies.requires) {
            if (dep == name) {
                printInvertTree(treeConfig, fullName, level + 1);
            }
        }
    }

    return
}

func printTreeStr(treeConfig: TreeConfig, name: String, level: Int32): Unit {
    for (_ in 0..level) {
        print("    ")
    }

    if (level != 0) {
        print("└── ${name}")
    } else {
        print("|-- ${name}")
    }

    if (treeConfig.isVerbose) {
        let version: String = treeConfig.deps[name].version
        if (!version.isEmpty()) {
            print(" ${version}")
        }
        let packagePath: String = treeConfig.deps[name].packagePath.allPaths[0].toString()
        if (!packagePath.isEmpty()) {
            print(" (${packagePath})")
        }
    }

    print("\n")
    return
}
