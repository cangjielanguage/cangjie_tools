// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.fs.*
import std.fs.remove as fsRemove
import std.posix.*
import std.env.*

@When[os == "Windows"]
public func deleteDirectory(dirPath: String): Bool {
    if (!directoryExists(dirPath)) {
        return true
    }

    // try five times when failed to delete directory in windows
    for (cnt in 0..5) {
        try {
            fsRemove(dirPath, recursive: true)
            break
        } catch (e: Exception) {
            if (cnt == 4) {
                eprintln(e.message)
                eprintln("Error: failed to delete the '${dirPath}' directory")
                return false
            }
            sleep(Duration.millisecond * 200)
        }
    }
    return true
}

@When[os != "Windows"]
public func deleteDirectory(dirPath: String): Bool {
    if (directoryExists(dirPath)) {
        try {
            fsRemove(dirPath, recursive: true)
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: failed to delete the '${dirPath}' directory")
            return false
        }
    }
    return true
}

public func createDirectory(dirPath: String): Bool {
    if (!directoryExists(dirPath)) {
        try {
            Directory.create(dirPath, recursive: true)
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: failed to create the '${dirPath}' directory")
            return false
        }
    }
    return true
}

@When[os == "Windows"]
public func deleteFile(filePath: String): Bool {
    if (!fileExists(filePath)) {
        return true
    }

    // try five times when failed to delete file in windows
    for (cnt in 0..5) {
        try {
            fsRemove(filePath)
            break
        } catch (e: Exception) {
            if (cnt == 4) {
                eprintln(e.message)
                eprintln("Error: failed to delete the '${filePath}' file")
                return false
            }
            sleep(Duration.millisecond * 200)
        }
    }
    return true
}

@When[os != "Windows"]
public func deleteFile(filePath: String): Bool {
    if (fileExists(filePath)) {
        try {
            fsRemove(filePath)
        } catch (e: Exception) {
            eprintln(e.message)
            eprintln("Error: failed to delete the '${filePath}' file")
            return false
        }
    }
    return true
}

public func getCanonicalPath(path: String): String {
    if (!directoryExists(path) && !fileExists(path)) {
        return ""
    }
    return canonicalize(Path(path)).toString().removeSuffix(PATH_SPLIT_CHAR)
}

public func getPath(prePath: String, curPath: String): String {
    if (curPath.isEmpty()) {
        return getCanonicalPath(prePath)
    }
    if (Path(curPath).isAbsolute()) {
        return curPath
    }
    var path = getCanonicalPath(Path(prePath).join(curPath).toString())
    if (path.isEmpty()) {
        return Path(prePath).join(curPath).toString()
    }
    return path
}

public func getAbsolutePath(path: String): (String, Bool) {
    if (path.size <= 0) {
        return ("", false)
    }
    let start = path.get(0)
    let c: Byte = start ?? UInt8(UInt32(r' '))
    var ppath = ""
    match (c) {
        case '~' =>
            let home: String = getVariable(HOME_ENV) ?? "/"
            ppath = replaceEnvVars(path.replace("~", home))
        case '.' =>
            let current = getcwd()
            ppath = replaceEnvVars(Path(current).join(path).toString())
        case _ => ppath = replaceEnvVars(path)
    }
    try {
        if (!directoryExists(ppath)) {
            Directory.create(ppath, recursive: true)
        }
        return (ppath, true)
    } catch (e: IllegalArgumentException) {
        eprintln("Illegal path: ${path}.")
        return (path, false)
    }
}

public func fileExists(path: String): Bool {
    if (!exists(path)) {
        return false
    } else if (FileInfo(path).isRegular()) {
        return true
    } else if (FileInfo(path).isSymbolicLink()) {
        return FileInfo(canonicalize(path)).isRegular()
    }
    return false
}

public func fileExists(path: Path): Bool {
    if (!exists(path)) {
        return false
    } else if (FileInfo(path).isRegular()) {
        return true
    } else if (FileInfo(path).isSymbolicLink()) {
        return FileInfo(canonicalize(path)).isRegular()
    }
    return false
}

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
