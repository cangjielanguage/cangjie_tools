// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.fs.exists as fs_exists
import std.collection.*
import std.fs.*

func directoryExists(path: String): Bool {
    if (!exists(path)) {
        return false
    } else if (FileInfo(path).isDirectory()) {
        return true
    } else if (FileInfo(path).isSymbolicLink()) {
        return FileInfo(canonicalize(path)).isDirectory()
    }
    return false
}

func getFileList(path: String): ArrayList<FileInfo> {
    let files = ArrayList<FileInfo>()
    Directory.walk(path) {
        info =>
        if (info.isRegular()) {
            files.add(info)
        }
        true
    }
    return files
}

func getDirectoryList(path: String): ArrayList<FileInfo> {
    let files = ArrayList<FileInfo>()
    Directory.walk(path) {
        info =>
        if (info.isDirectory()) {
            files.add(info)
        }
        true
    }
    return files
}
