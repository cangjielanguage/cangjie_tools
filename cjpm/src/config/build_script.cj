// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*

public const BUILD_SCRIPT_NAME: String = "build.cj"
public const BUILD_SCRIPT_DIR: String = "build-script-cache"
public const BUILD_SCRIPT_CACHE: String = ".script-hash"
public const BUILD_SCRIPT_LOG: String = "script-log"

public enum BuildScriptStep <: ToString & Equatable<BuildScriptStep> {
    | Pre
    | Post

    public func toString(): String {
        match (this) {
            case Pre => "pre"
            case Post => "post"
        }
    }

    public operator func ==(other: BuildScriptStep): Bool {
        match ((this, other)) {
            case (Pre, Pre) => true
            case (Post, Post) => true
            case _ => false
        }
    }
    public operator func !=(other: BuildScriptStep): Bool {
        !(this == other)
    }
}

public let BUILD_SCRIPT_CMD_PRE: Array<String> = ["check", "update", "tree", "run", "clean"]
public let BUILD_SCRIPT_CMD_POST: Array<String> = ["check", "update", "tree", "run"]
public let DEBUG_CMD: Array<String> = ["run", "clean"]
public let VERBOSE_CMD: Array<String> = ["tree", "run"]
public let CROSS_CMD: Array<String> = ["tree"]
@When[os == "Linux" || os == "macOS"]
public const BUILD_SCRIPT_OUTPUT: String = "build-script"
@When[os == "Windows"]
public const BUILD_SCRIPT_OUTPUT: String = "build-script.exe"
public let RELOCATED_SCRIPT_MAP: HashSet<String> = HashSet<String>()
// command, exe path, env map
public let RUN_SCRIPT_CMD_MAP: HashMap<String, (String, String, HashMap<String, String>)> = HashMap<String,
    (String, String, HashMap<String, String>)>()

