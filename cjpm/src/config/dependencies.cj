// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.collection.*
import std.fs.*
import stdx.serialization.serialization.*

const TYPE_ERR_INFO: String = "  - incompatible import type, only one of 'path' or 'git' is allowed"

public class DepInfo <: Serializable<DepInfo> & Equatable<DepInfo> {
    public var path: ?String = None
    public var git: ?String = None
    public var commitId: ?String = None
    public var branch: ?String = None
    public var tag: ?String = None
    public var outputType: ?String = None
    public var pathInWorkspace: ?String = None
    public var pathDepInGit: Bool = false
    public var importedPath: String = ""

    public init() {}

    public init(path: String) {
        this.path = path
    }

    public func serialize(): DataModel {
        DataModelStruct()
            .addFieldIfSome("path", path)
            .addFieldIfSome("git", git)
            .addFieldIfSome("commitId", commitId)
            .addFieldIfSome("branch", branch)
            .addFieldIfSome("tag", tag)
            .addFieldIfSome(OUTPUT_TYPE, outputType)
    }

    public static func deserialize(dm: DataModel): DepInfo {
        let result = DepInfo()
        match (dm) {
            case data: DataModelStruct =>
                result.path = Option<String>.deserialize(data.get("path"))
                result.git = Option<String>.deserialize(data.get("git"))
                result.commitId = Option<String>.deserialize(data.get("commitId"))
                result.branch = Option<String>.deserialize(data.get("branch"))
                result.tag = Option<String>.deserialize(data.get("tag"))
                result.outputType = Option<String>.deserialize(data.get(OUTPUT_TYPE))
            case _ => throw Exception("this data is not DataModelStruct")
        }
        return result
    }

    public func isPathDep(): Bool {
        return this.path.isSome()
    }

    public func isGitDep(): Bool {
        return this.git.isSome()
    }

    func isSameDep(other: DepInfo): String {
        if (this.isPathDep()) {
            if (this.path == other.path) {
                return ""
            }
            if (this.pathInWorkspace.isSome() && this.pathInWorkspace == other.path) {
                return ""
            }
            if (other.pathInWorkspace.isSome() && this.path == other.pathInWorkspace) {
                return ""
            }
            return "  - ${this.path.getOrThrow()}\n  - ${other.path.getOrThrow()}"
        } else {
            let sameBranchSpecified = this.branch == other.branch
            let sameTagSpecified = this.tag == other.tag
            // for commitId it is ok to have one of them unspecified because it can be saved only in lock file
            let sameCommitSpecified = this.commitId == other.commitId || this.commitId.isSome() !=
                other.commitId.isSome()
            let sameGitSpecified = this.git.isSome() && checkGitUrl(this.git, other.git) && sameCommitSpecified &&
                sameBranchSpecified && sameTagSpecified
            if (this.isGitDep() && !sameGitSpecified) {
                return "  - incompatible git dependency"
            }
            return ""
        }
    }

    // Check if two git url is same
    func checkGitUrl(u1: ?String, u2: ?String): Bool {
        if ((let Some(v1) <- u1) && (let Some(v2) <- u2)) {
            return v1.removeSuffix(".git") == v2.removeSuffix(".git")
        }
        return false
    }

    public func isAlternative(other: DepInfo): (Bool, String) {
        if (this.outputType.isSome() && other.outputType.isSome() && this.outputType != other.outputType) {
            return (false, "  - incompatible output type")
        }

        if (this.isPathDep()) {
            // Current dependency is a local dependency
            if (other.isPathDep()) {
                return (false, this.isSameDep(other))
            } else {
                return (false, TYPE_ERR_INFO)
            }
        } else {
            // Current dependency is a git dependency
            if (other.isPathDep()) {
                return (false, TYPE_ERR_INFO)
            } else {
                return (false, this.isSameDep(other))
            }
        }
    }

    public func updateWith(other: DepInfo): Unit {
        this.path = this.path.updatedWith(other.path)
        this.git = this.git.updatedWith(other.git)
        this.commitId = this.commitId.updatedWith(other.commitId)
        this.branch = this.branch.updatedWith(other.branch)
        this.tag = this.tag.updatedWith(other.tag)
        this.outputType = this.outputType.updatedWith(other.outputType)
        this.pathDepInGit = other.pathDepInGit
    }

    public operator func ==(other: DepInfo): Bool {
        this.path == other.path && this.git == other.git && this.commitId == other.commitId &&
            this.branch == other.branch && this.tag == other.tag && this.outputType == other.outputType
    }

    public operator func !=(other: DepInfo): Bool {
        !(this == other)
    }
}

extend DataModelStruct {
    func addFieldIfSome<T>(name: String, val: ?T): DataModelStruct where T <: Serializable<T> {
        match (val) {
            case Some(f) => this.add(field(name, f))
            case None => ()
        }
        this
    }
}

extend<T> Option<T> {
    public func updatedWith(other: ?T): Option<T> {
        if (other.isSome()) {
            other
        } else {
            this
        }
    }
}

public func verifyDependencyConfig(requires: HashMap<String, DepInfo>, errLoc: String, modulePath: String): Bool {
    var checkFlag: Bool = true
    for ((name, v) in requires) {
        let checkList: Array<((String, String) -> Bool, ?String)> = [(safeCheck, v.commitId), (nameCheck, name)]

        if (!verifyAll(errLoc, checkList)) {
            checkFlag = false
        }

        if (let Some(val) <- v.path) {
            if (!depPathCheck(val, errLoc, modulePath, name)) {
                checkFlag = false
            }
        }

        if (!verifyDependencyField(v, errLoc)) {
            checkFlag = false
        }

        if (let Some(val) <- v.outputType) {
            if (val != STATIC_TYPE && val != DYNAMIC_TYPE) {
                eprintln("Error: field 'output-type' in dependency '${name}' can only be 'static' or 'dynamic', " +
                    "please check ${errLoc}")
                checkFlag = false
            }
        }
    }
    return checkFlag
}

public func verifyDependencyField(dep: DepInfo, errLoc: String): Bool {
    if (dep.path.isNone() && dep.git.isNone()) {
        eprintln("Error: one of 'git' or 'path' fields must exist in ${errLoc}")
        return false
    }
    if (dep.path.isSome() && dep.git.isSome()) {
        eprintln("Error: only one of 'git' or 'path' fields can exist in ${errLoc}")
        return false
    }
    return true
}

func verifyDependencyField(dep: DepInfo, name: String, path: String): Bool {
    if (dep.path.isNone() && dep.git.isNone()) {
        eprintln(
            "Error: one of 'git' or 'path' fields must exist " +
                "in dependency ${name} in ${Path(path).join(CONFIG_FILE_NAME).toString()}")
        return false
    }
    if (dep.path.isSome() && dep.git.isSome()) {
        eprintln(
            "Error: only one of 'git' or 'path' fields can exist " +
                "in dependency ${name} in ${Path(path).join(CONFIG_FILE_NAME).toString()}")
        return false
    }
    return true
}
