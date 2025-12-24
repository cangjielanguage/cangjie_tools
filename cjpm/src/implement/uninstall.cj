// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import cjpm.config.*

// Command uninstall implement.
public func doUninstall(root: String, uninstallList: ArrayList<String>): Bool {
    var res: Bool = false
    if (let Some(val) <- loadInstallList(root)) {
        installedBinList = val
    } else {
        eprintln("Error: The uninstall root path `${root}` does not exist or nothing is installed here")
        return res
    }
    var isSuccessedUninstall = false
    for (binName in uninstallList) {
        var uninstallRes = uninstallBinary(root, binName)
        res ||= uninstallRes
        if (!uninstallRes) {
            eprintln("Error: failed to uninstall ${binName}")
        } else {
            isSuccessedUninstall = true
        }
    }
    if (isSuccessedUninstall) {
        saveInstallList(root)
    }
    return res
}

func uninstallBinary(root: String, binName: String): Bool {
    if (!installedBinList.contains(binName)) {
        eprintln("Error: ${binName} is not installed")
        return false
    }
    let binPath = Path(root).join(BIN).toString()
    let libPath = Path(root).join("libs").toString()
    if (!deleteFile(Path(binPath).join(makeExeName(binName)).toString()) ||
        !deleteDirectory(Path(libPath).join(binName).toString())) {
        return false
    }
    installedBinList.remove(binName)
    println("successfully uninstalled ${binName}")
    return true
}
