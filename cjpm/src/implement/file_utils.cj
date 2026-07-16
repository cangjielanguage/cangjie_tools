// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*

// Walk path once and collect entries accepted by predicate.
// If path is a symbolic link, walk its canonical target to preserve the existing behavior.
func getEntryList(path: String, predicate: (FileInfo) -> Bool): ArrayList<FileInfo> {
    let entries = ArrayList<FileInfo>()
    // Handle symbolic links: resolve to real path first
    let realPath = if (FileInfo(Path(path)).isSymbolicLink()) {
        canonicalize(Path(path)).toString()
    } else {
        path
    }
    Directory.walk(realPath) {
        info =>
        if (predicate(info)) {
            entries.add(info)
        }
        true
    }
    return entries
}

func getFileList(path: String): ArrayList<FileInfo> {
    return getEntryList(path) { info => info.isRegular() }
}

func getDirectoryList(path: String): ArrayList<FileInfo> {
    return getEntryList(path) { info => info.isDirectory() }
}
