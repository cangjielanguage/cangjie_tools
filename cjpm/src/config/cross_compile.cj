// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

public func replaceNativeInfo(oriInfo: TargetInfo, newInfo: TargetInfo): Unit {
    oriInfo.compileOption = combineOptionString(oriInfo.compileOption, newInfo.compileOption)
    oriInfo.overrideCompileOption = combineOptionString(oriInfo.overrideCompileOption, newInfo.overrideCompileOption)
    oriInfo.linkOption = combineOptionString(oriInfo.linkOption, newInfo.linkOption)
    return
}

public func makeTargetDylibName(fileName: String): String {
    if (crossCompileTarget.contains("mingw32") || crossCompileTarget.contains("windows-gnu")) {
        return "lib${fileName}.dll"
    } else if (crossCompileTarget.contains("apple-ios") || crossCompileTarget.contains("darwin")) {
        return "lib${fileName}.dylib"
    } else {
        return "lib${fileName}.so"
    }
}

public func makeTargetExeName(exeName: String): String {
    if (crossCompileTarget.contains("mingw32") || crossCompileTarget.contains("windows-gnu")) {
        if (!exeName.endsWith(".exe")) {
            return "${exeName}.exe"
        }
    }
    return exeName
}
