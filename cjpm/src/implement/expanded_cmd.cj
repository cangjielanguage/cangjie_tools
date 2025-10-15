// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.fs.*
import std.collection.*
import std.env.*
import cjpm.config.*

let CJPM_PREFIX: String = "cjpm-"
@When[os == "Linux" || os == "macOS"]
let CJPM_POSTFIX: String = ""
@When[os == "Linux" || os == "macOS"]
let ENV_PATH_SPLIT_CHAR: String = ":"
@When[os == "Windows"]
let CJPM_POSTFIX: String = ".exe"
@When[os == "Windows"]
let ENV_PATH_SPLIT_CHAR: String = ";"

func runExpandedCmd(arguments: ArrayList<String>): Bool {
    let exeTool: String = arguments[0]
    let newArgs: ArrayList<String> = ArrayList<String>(arguments)
    newArgs.remove(at: 0)
    return execAndToTerminal(exeTool, newArgs.toArray())
}

func checkPath(fileName: String): Option<String> {
    var pathStr: String = getVariable("PATH") ?? ""
    if (pathStr.isEmpty()) {
        return None
    }
    let pathList: Array<String> = pathStr.split(ENV_PATH_SPLIT_CHAR)
    for (path in pathList) {
        var fileList: ArrayList<FileInfo> = ArrayList<FileInfo>()
        try {
            if (!directoryExists(path)) {
                continue
            }
            fileList = getFileList(path)
        } catch (_: FSException | IllegalArgumentException) {
            continue
        }
        for (fileInfo in fileList) {
            let filePath: String = fileInfo.path.toString()
            let filePathArray: Array<String> = filePath.split(PATH_SPLIT_CHAR)
            if (!filePathArray.isEmpty() && (filePathArray[filePathArray.size - 1] == fileName)) {
                return filePath
            }
        }
    }
    return None
}

public func checkExpandedCmd(arguments: ArrayList<String>): Bool {
    var fileName: String = CJPM_PREFIX + arguments[0] + CJPM_POSTFIX
    let filePath: String = checkPath(fileName) ?? ""
    if (!filePath.isEmpty()) {
        arguments[0] = filePath
        return runExpandedCmd(arguments)
    }
    eprintln("Error: unknown command '${arguments[0]}' for cjpm")
    return false
}
