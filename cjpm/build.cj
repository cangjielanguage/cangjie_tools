// Copyright (c) Huawei Technologies Co., Ltd. 2026. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

import std.process.{Process, executeWithOutput}
import std.fs.Path

let NATIVE_BUILD_ARGS = ["build.py", "build", "-t", "release", "--native-only"]

let CROSS_WIN_BUILD_ARGS = ["build.py", "build", "-t", "release", "--target", "windows-x86_64", "--native-only"]

@When[os == "Windows"]
func isWindows() {
    true
}

@When[os != "Windows"]
func isWindows() {
    false
}

@When[target != "x86_64-w64-mingw32"]
func stagePreBuild(): Int64 {
    let (returnCode, stdOut, stdError) = executeWithOutput("python3", NATIVE_BUILD_ARGS, workingDirectory: Path("./build"))
    println(returnCode.toString())
    println(String.fromUtf8(stdOut))
    println(String.fromUtf8(stdError))
    0
}

@When[target == "x86_64-w64-mingw32"]
func stagePreBuild(): Int64 {
    let args = if (isWindows()) {
        NATIVE_BUILD_ARGS
    } else {
        CROSS_WIN_BUILD_ARGS
    }
    let (returnCode, stdOut, stdError) = executeWithOutput("python3", args, workingDirectory: Path("./build"))
    println(returnCode.toString())
    println(String.fromUtf8(stdOut))
    println(String.fromUtf8(stdError))
    0
}

func stagePreClean(): Int64 {
    executeWithOutput("python3", ["build.py", "clean"], workingDirectory: Path("./build"))
    0
}

main(): Int64 {
    match (Process.current.arguments[0]) {
        case "pre-build" => stagePreBuild()
        case "pre-clean" => stagePreClean()
        case _ => 0
    }
}