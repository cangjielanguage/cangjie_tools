// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

// Package link argument builders shared by normal build, module combine, and project lto-combine.
// Used from build.cj (getBuildArgs/getForcedLibArgs), test.cj, and lto-combine root linking.

// iosCombineBitcodeOnly: iOS lto-combine root — link .bc only, no .a/-L/-l fallback.
func linkPackageImpl(libName: String, libPath: String, isLto: Bool, iosCombineBitcodeOnly: Bool): ArrayList<String> {
    let processedLibName = swapOrgName(libName)
    let res = ArrayList<String>()
    let ltoFile = Path(libPath).join(makeLtoName(processedLibName)).toString()
    if (fileExists(ltoFile) && (iosCombineBitcodeOnly || isLto)) {
        res.add(ltoFile)
        return res
    }
    if (iosCombineBitcodeOnly) {
        return res
    }

    if (fileExists(Path(libPath).join(makeCangjieStaticlibName(processedLibName))) ||
        fileExists(Path(libPath).join("lib${processedLibName}.so")) ||
        fileExists(Path(libPath).join("lib${processedLibName}.dll")) ||
        fileExists(Path(libPath).join("lib${processedLibName}.dylib"))) {
        res.add(all: ["-L", libPath, "-l${processedLibName}"])
    } else if (fileExists(ltoFile)) {
        res.add(ltoFile)
    }

    return res
}

// Build package link arguments according to the compile task's LTO and iOS combine mode.
public func linkPackage(libName: String, libPath: String, compileTask: CompileTask): ArrayList<String> {
    linkPackageImpl(libName, libPath, compileTask.combineFlags.isLto, isIosLtoCombineLinkTask(compileTask))
}

// For test / cross-compile test paths that have no CompileTask.
public func linkPackage(libName: String, libPath: String, isLto: Bool): ArrayList<String> {
    linkPackageImpl(libName, libPath, isLto, false)
}

// Select the platform-specific forced-link implementation for a package.
public func forcedLinkPackage(libName: String, libPath: String, compileTask: CompileTask): ArrayList<String> {
    let compileTarget = getCompileTarget()
    if (compileTarget.contains("darwin") || compileTarget.contains("apple")) {
        return forcedLinkPackageForMac(libName, libPath, compileTask)
    } else {
        return forcedLinkPackageForLinuxOrWin(libName, libPath, compileTask)
    }
}

// Build forced-link arguments for Linux and Windows targets.
public func forcedLinkPackageForLinuxOrWin(libName: String, libPath: String, compileTask: CompileTask): ArrayList<String> {
    return linkPackage(libName, libPath, compileTask)
}

// Build forced-link arguments for Apple targets, preferring native static libraries.
public func forcedLinkPackageForMac(libName: String, libPath: String, compileTask: CompileTask): ArrayList<String> {
    if (isIosModuleLtoCombineRootCompileTask(compileTask)) {
        return linkPackage(libName, libPath, compileTask)
    }
    var res = ArrayList<String>()
    let processedLibName = swapOrgName(libName)
    let staticLibPath = Path(libPath).join(makeCangjieStaticlibName(processedLibName)).toString()
    let dyLibPath = Path(libPath).join("lib${processedLibName}.dylib").toString()
    if (fileExists(staticLibPath)) {
        res.add(staticLibPath)
    } else if (fileExists(dyLibPath)) {
        res.add(all: linkPackage(libName, libPath, compileTask))
    }
    return res
}

// Collect linker options for binary dependencies selected by the package set.
func appendOptionForBindep(binDepMap: HashMap<String, BinDeps>, linkPkgs: HashSet<String>,
    compileTask: CompileTask): ArrayList<String> {
    let (libPathOptions, _) = getLinkOptionForBindep(binDepMap, linkPkgs, compileTask)
    libPathOptions
}

// Resolve package and standard-library linker options for binary dependencies.
func getLinkOptionForBindep(binDepMap: HashMap<String, BinDeps>, linkPkgs: HashSet<String>,
    compileTask: CompileTask): (ArrayList<String>, StdDeps) {
    let linkerOptions = ArrayList<String>()
    let libPathOptions = ArrayList<String>()
    let moduleLinkerOptionsList = ArrayList<ArrayList<String>>()
    let stdDeps = StdDeps()
    for ((_, eachModuleDepMap) in binDepMap) {
        let moduleLinkerOptions = ArrayList<String>()
        for (index in (eachModuleDepMap.packageRequiresSort.size - 1)..=0 : -1) {
            let depName = eachModuleDepMap.packageRequiresSort[index]
            if (!linkPkgs.contains(depName)) {
                continue
            }
            let info = eachModuleDepMap.packageRequires[depName]
            if (info.isMacroPackage) {
                continue
            }
            stdDeps.add(all: info.stdRequires)
            let linkArray: ArrayList<String> = linkPackage(info.libName, info.libPath, compileTask)

            if (linkArray.size <= 1) {
                moduleLinkerOptions.add(all: linkArray)
                continue
            }

            if (!libPathOptions.contains(linkArray[1])) {
                libPathOptions.add(linkArray[0])
                libPathOptions.add(linkArray[1])
            }
            moduleLinkerOptions.add(linkArray[2])
        }

        if (!moduleLinkerOptionsList.contains(moduleLinkerOptions)) {
            moduleLinkerOptionsList.add(moduleLinkerOptions)
            linkerOptions.add(all: moduleLinkerOptions)
        }
    }

    libPathOptions.add(all: linkerOptions)
    (libPathOptions, stdDeps)
}

// Gather C-library linker fragments for the requested package names.
func getCLibLinkStr(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>): ArrayList<String> {
    var res = ArrayList<String>()
    let cLibNameSet = HashSet<String>(cLibNameList)
    for (cLibName in cLibNameSet) {
        if (cLibLinkMap.contains(cLibName)) {
            res.add(all: cLibLinkMap[cLibName])
        }
    }
    return res
}

// Convert C-library linker fragments into forced-link arguments.
func getForcedCLibLinkOptions(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>,
    compileTask: CompileTask): ArrayList<String> {
    let res = ArrayList<String>()
    var libName = ""
    var libPath = ""
    for (part in getCLibLinkStr(cLibLinkMap, cLibNameList)) {
        if (part == "-L") {
            continue
        } else if (part.startsWith("-l")) {
            libName = part.removePrefix("-l")
        } else {
            libPath = part
        }
        if (!libName.isEmpty() && !libPath.isEmpty()) {
            res.add(all: forcedLinkPackage(libName, libPath, compileTask))
            libName = ""
            libPath = ""
        }
    }
    return res
}

// Deduplicate C-library search paths and library names while preserving order.
func getMinimizedCLibLinkStr(cLibLinkMap: HashMap<String, ArrayList<String>>, cLibNameList: ArrayList<String>): ArrayList<String> {
    let cLibLinkStr = getCLibLinkStr(cLibLinkMap, cLibNameList)

    let res = ArrayList<String>()

    var index: Int64 = 0
    let cLibPathSet: HashSet<String> = HashSet<String>()
    let cLibNameSet: HashSet<String> = HashSet<String>()
    while (index < cLibLinkStr.size) {
        if (cLibLinkStr[index] == "-L") {
            if (index + 1 < cLibLinkStr.size && !cLibPathSet.contains(cLibLinkStr[index + 1])) {
                res.add(cLibLinkStr[index])
                res.add(cLibLinkStr[index + 1])
                cLibPathSet.add(cLibLinkStr[index + 1])
            }
            index += 2
        } else if (cLibLinkStr[index].startsWith("-l")) {
            if (!cLibNameSet.contains(cLibLinkStr[index])) {
                res.add(cLibLinkStr[index])
                cLibNameSet.add(cLibLinkStr[index])
            }
            index += 1
        } else {
            res.add(cLibLinkStr[index])
            index += 1
        }
    }

    return res
}

// Select the native or cross-compilation C-library link map for a compile task.
func getCLibLinkMap(compileTask: CompileTask, buildConfig: BuildConfig): HashMap<String, ArrayList<String>> {
    if (compileTask.isCrossCompile) {
        buildConfig.globalConfig.crossCLibLinkMap
    } else {
        buildConfig.globalConfig.cLibLinkMap
    }
}

// Append cLib for forced-link flows. Skipped on iOS lto-combine roots (linked in Xcode).
func appendForcedCLibLinkOptions(options: ArrayList<String>, compileTask: CompileTask, buildConfig: BuildConfig,
    cLibNameList: ArrayList<String>): Unit {
    if (isIosLtoCombineLinkTask(compileTask)) {
        return
    }
    let cLibLinkMap = getCLibLinkMap(compileTask, buildConfig)
    let compileTarget = getCompileTarget()
    if (compileTarget.contains("darwin") || compileTarget.contains("apple")) {
        options.add(all: getForcedCLibLinkOptions(cLibLinkMap, cLibNameList, compileTask))
    } else {
        options.add(all: getMinimizedCLibLinkStr(cLibLinkMap, cLibNameList))
    }
}

// Link arguments for project lto-combine root (combine-all-deps).
// Links all LTO bitcode deps, then bin-deps; iOS skips native cLib (linked in Xcode).
func getLtoCombineBuildArgs(compileTask: CompileTask, buildConfig: BuildConfig): ArrayList<String> {
    let res = ArrayList<String>()
    let cLibNameList = ArrayList<String>([compileTask.rootPkgName])
    let linkSort: HashSet<String> = getDepSet(compileTask, buildConfig)
    let allOptions = ArrayList<String>()
    let libPathSet = HashSet<String>()
    let binDepSet = HashSet<String>()
    for (index in (buildConfig.packageList.all.size - 1)..=0 : -1) {
        let depName = buildConfig.packageList.all[index]
        if (linkSort.contains(depName)) {
            linkSort.add(all: getDepSet(depName, buildConfig))
        }
        let depRootName: String = getRootPkgName(depName)
        cLibNameList.add(depRootName, at: 0)
        if (buildConfig.packageList.isMacro(depName) || buildConfig.packageList.isTest(depName)) {
            continue
        }

        var libPath = Path(compileTask.targetDir).join(swapOrgName(depRootName)).toString()

        let linkArray: ArrayList<String> = linkPackage(depName, libPath, compileTask)
        if (linkArray.size <= 1) {
            allOptions.add(all: linkArray)
            binDepSet.add(all: getDepSet(depName, buildConfig))
            continue
        }
        if (libPathSet.contains(libPath)) {
            allOptions.add(linkArray[linkArray.size - 1])
        } else {
            allOptions.add(all: linkArray)
            libPathSet.add(libPath)
        }
        binDepSet.add(all: getDepSet(depName, buildConfig))
    }
    res.add(all: allOptions)

    let binOptions = ArrayList<String>()
    binOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, linkSort, compileTask), at: 0)
    binOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, linkSort, compileTask), at: 0)
    res.add(all: binOptions)

    let otherBinOptions = ArrayList<String>()
    binDepSet.remove(all: linkSort)
    let compileTarget = compileTask.target
    if (isIosProjectCombineCompileTask(compileTask)) {
        otherBinOptions.add(all: appendOptionForBindep(buildConfig.binDepMap, binDepSet, compileTask), at: 0)
        otherBinOptions.add(all: appendOptionForBindep(buildConfig.crossBinDepMap, binDepSet, compileTask), at: 0)
        res.add(all: otherBinOptions)
    } else {
        otherBinOptions.add(all: getForcedBinDep(compileTask, buildConfig, binDepSet))
        appendForcedCLibLinkOptions(otherBinOptions, compileTask, buildConfig, cLibNameList)
        addForcedLibArgs(res, otherBinOptions, compileTarget)
    }

    return res
}

// Extra cjc flags for iOS lto-combine root native staticlib output (project or module).
func getIosLtoCombineNativeStaticlibCompileArgs(): ArrayList<String> {
    let res = ArrayList<String>()
    res.add(all: extractOptionByString("--lto-staticlib-format=native"))
    res
}
