// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import std.regex.*
import std.env.*
import cjpm.config.*

const WINDOWS_LOCAL_FILE_REGEX = "^[A-Za-z]:.+$"
const URL_WITH_SPACE = "^.*\\s+.*$"

public class GitCli {
    func getRemoteRef(url: String, ref: String): ?String {
        setVariable("GIT_TERMINAL_PROMPT", "0")
        setVariable("GIT_SSH_COMMAND", "ssh -o ConnectTimeout=5 -o BatchMode=yes -o StrictHostKeyChecking=ask")

        let (success, outInfo, errInfo) = execWithOutput("git", ArrayList<String>(["ls-remote", url, ref]),
            timeout: Duration.second * 30)
        if (!success) {
            eprintln("Error invoking git:")
            eprintln(errInfo + outInfo)
            return None
        }

        // A bit weird regex used here because Cangjie regex engine currently handles match group indexes incorrectly
        let gitRefsParser = Regex("^([0-9a-f]+)\\s+(?:refs\\/(?:heads|tags)\\/)?([^\\s]+)$", MultiLine)
        let matchDataArray = gitRefsParser.findAll(outInfo, group: true)

        for (next in matchDataArray) {
            if (next.matchString(2) == ref) {
                return next.matchString(1)
            }
        }

        return None
    }

    public func clone(url: String, targetDir: String, hash: String): Bool {
        setVariable("GIT_TERMINAL_PROMPT", "0")
        setVariable("GIT_DIR", Path(targetDir).join(".git").toString())
        setVariable("GIT_WORK_TREE", targetDir)

        // same as `git clone` but allows to fetch by commit hash
        let error = execAndGetError("init") ?? execAndGetError("remote add --no-tags origin ${url}") ??
            execAndGetError("fetch --depth=1 origin ${hash}") ?? execAndGetError("checkout --force ${hash}") ??
            return true

        eprintln("Error invoking git:")
        eprintln(error)
        return false
    }

    // returns Some if unsuccessfull
    func execAndGetError(command: String): ?String {
        let arguments = extractOptionByString(command)
        let (success, outInfo, errInfo) = execWithOutput("git", arguments)
        if (success) {
            return Option.None
        } else {
            return errInfo + outInfo
        }
    }

    func getCommitHashFromName(url: String, name: ?String): ?String {
        let target = name ?? "HEAD"
        let hash = getRemoteRef(url, target)
        if (hash.isNone()) {
            eprintln("Error: reference '${target}' not found in git repository with url: ${url}")
        }
        return hash
    }

    public func isAvailable(): Bool {
        let (success, _, _) = execWithOutput(WHICH, ArrayList<String>(["git"]))
        return success
    }
}

func checkRemoteUrl(url: String): Bool {
    let trimmedUrl: String = url.trimAscii()
    let isLegal: Bool = trimmedUrl.startsWith("/") || trimmedUrl.startsWith("\\\\") ||
        trimmedUrl.startsWith("file:") || trimmedUrl.startsWith("git:") || trimmedUrl.startsWith("git@") ||
        trimmedUrl.startsWith("http:") || trimmedUrl.startsWith("https:") || trimmedUrl.startsWith("ssh:") ||
        Regex(WINDOWS_LOCAL_FILE_REGEX).matches(trimmedUrl)
    let isUnsafe: Bool = trimmedUrl.startsWith("-") || trimmedUrl.contains("`") ||
        trimmedUrl.contains("--upload-pack=") || Regex(URL_WITH_SPACE).matches(trimmedUrl)
    if (!isLegal && isUnsafe) {
        eprintln("Error: invalid remote url: '${url}'")
        return false
    }
    return true
}

public func downloadGitDep(name: String, v: DepInfo): ?(String, String, Bool) {
    let cjpmGlobalDir = getGlobalFolderPath()
    let gitStoragePath = cjpmGlobalDir.join("git")

    let git = GitCli()

    if (!git.isAvailable()) {
        eprintln("Error: git is not available. Please install it or make it available through $PATH.")
        return None
    }

    let url = v.git ?? return None
    if (!checkRemoteUrl(url)) {
        return None
    }
    let targetCommit = v.commitId ?? git.getCommitHashFromName(url, v.branch ?? v.tag) ?? return None

    let dependencyPath = gitStoragePath.join(name).join(targetCommit).toString()

    if (directoryExists(dependencyPath)) {
        return (dependencyPath, targetCommit, false)
    } else {
        try {
            Directory.create(dependencyPath, recursive: true)
        } catch (e: FSException) {
            eprintln("Error while working with cjpm shared directory(`${cjpmGlobalDir}`): ${e.message}")
        }

        let result = git.clone(url, dependencyPath, targetCommit)
        if (result) {
            return checkGitWorkspace(url, dependencyPath, targetCommit, gitStoragePath)
        }

        deleteDirectory(dependencyPath)
        return None
    }
}
