// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.config

import std.regex.*
import std.fs.*

// Regexp root package name.
let EXAMPLE_ROOT_PACKAGE_NAME: String = "cjDemo"
let EXAMPLE_ENV_NAME: String = "CJ_ENV_NAME"
// Regexp version.
public let REGEXP_VERTION: String = "^(([1-9][0-9]+)|[0-9])\\.(([1-9][0-9]+)|[0-9])\\.(([1-9][0-9]+)|[0-9])$"
let EXAMPLE_MODULE_VERSION: String = "1.0.0"
// used for central repository
let REGEXP_NAME_VERSION: String = "^(_*[a-zA-Z][_a-zA-Z0-9]*):(([1-9][0-9]+)|[0-9])\\.(([1-9][0-9]+)|[0-9])\\.(([1-9][0-9]+)|[0-9])$"
// used for central repository
public let EXAMPLE_NAME_VERSION: String = "cjModDemo:1.0.0"
let EXAMPLE_CJC_VERSION: String = "1.0.0"
// regexp c library name.
let REGEXP_C_LIB_NAME: String = "^[A-Za-z0-9\\._\\-#%\\+,=@\\[\\]\\^\\{\\}~]+$"
// Regexp condition value.
let REGEXP_CONDITON_VALUE: String = "^_*[a-zA-Z][_a-zA-Z0-9]*$"
let EXAMPLE_CONDITON_VALUE: String = "cfg_1"

// Regexp command injection.
@When[os == "Linux" || os == "macOS"]
let REGEXP_COMMAND_INJECTION = "[|;&$><`!\n\\\\]+"
@When[os == "Windows"]
let REGEXP_COMMAND_INJECTION = "[|;&$><`!\n]+"

// Check root package name in cjpm.toml.
public func nameCheck(text: String, path: String): Bool {
    if (text.size == 0) {
        eprintln("Error: the 'name' field in ${path} can't be empty")
        return false
    }
    var matched = Regex(REGEXP_CONDITON_VALUE)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: the 'name' field '${text}' in ${path} is wrong, " +
            "the example is '${EXAMPLE_ROOT_PACKAGE_NAME}'")
    }
    return false
}

// Check initial root package name.
public func nameCheck(text: String): Bool {
    if (text.size == 0) {
        eprintln("Error: the 'name' field can't be empty")
        return false
    }
    var matched = Regex(REGEXP_CONDITON_VALUE)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: the 'name' field '${text}' is wrong, " +
            "the example is '${EXAMPLE_ROOT_PACKAGE_NAME}'")
    }
    return false
}

func cLibNameCheck(text: String, path: String): Bool {
    if (text.size == 0) {
        eprintln("Error: the c-library name can't be empty which is listed in 'ffi.c' field at ${path}")
        return false
    }
    let matched = Regex(REGEXP_C_LIB_NAME)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: the c-library name '${text}' is wrong " +
            "which is listed in 'ffi.c' field at ${path}")
    }
    return false
}

// Check version in cjpm.toml.
func versionCheck(text: String, path: String): Bool {
    if (text.isEmpty()) {
        eprintln("Error: the 'version' field in ${path} can't be empty")
        return false
    }
    let matched = Regex(REGEXP_VERTION)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: the 'version' field '${text}' in ${path} is wrong, " +
            "the example is '${EXAMPLE_MODULE_VERSION}'")
    }
    return false
}

// used for central repository
// Check name:version args
public func nameVersionCheck(text: String): Bool {
    let matched = Regex(REGEXP_NAME_VERSION)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => return false
    }
}

// used for central repository
// Check central dependencies version in cjpm.toml.
func centralVersionCheck(name: String, text: String, path: String): Bool {
    if (text.isEmpty()) {
        eprintln("Error: name of dependency from central repository in ${path} can't be empty")
        return false
    }
    let matched = Regex(REGEXP_VERTION)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: version '${text}' for central repository dependency '${name}' in ${path} " +
            "is wrong, the example is '${EXAMPLE_MODULE_VERSION}'")
    }
    return false
}

// Check cjc_version in cjpm.toml.
func cjcVersionCheck(text: String, path: String): Bool {
    if (text.isEmpty()) {
        eprintln("Error: the '${FILE_VERSION}' field in ${path} can't be empty")
        return false
    }
    let matched = Regex(REGEXP_VERTION)
    match (matched.find(text)) {
        case Some(_) => return true
        case None => eprintln("Error: the '${FILE_VERSION}' field '${text}' in ${path} is wrong, " +
            "the example is '${EXAMPLE_CJC_VERSION}'")
    }
    return false
}

// Check output_type in cjpm.toml.
func outputTypeCheck(text: String, path: String): Bool {
    if (let Unknown(_) <- OutputType.fromString(text)) {
        eprintln("Error: the 'output-type' field in ${path} is wrong, optional value are '${Static}/${Dynamic}/${Exe}'.")
        return false
    }
    return true
}

// Check the 'lto' field in cjpm.toml.
@When[os == "Linux"]
public func ltoValueCheck(text: String, path: String): Bool {
    if (text.isEmpty()) {
        return true
    }

    if (!LTO_TYPE_LIST.contains(text)) {
        let options: String = String.join(LTO_TYPE_LIST, delimiter: "/")
        eprintln("Error: the 'lto' field in ${path} is wrong, optional value are '${options}'")
        return false
    }

    return true
}

@When[os != "Linux"]
public func ltoValueCheck(_: String, _: String): Bool {
    return true
}

public func safeCheck(text: String, path: String): Bool {
    if (let Some(err) <- safeCheck_valueError(text, path)) {
        eprintln(err)
        return false
    }
    return true
}

public func safeCheck_valueError(text: String, path: String): ?String {
    let hasPath: Bool = !path.isEmpty()
    if (Regex(REGEXP_COMMAND_INJECTION).matches(text)) {
        var errorString: String = "Error: the '${text}' contains illegal characters"
        if (hasPath) {
            errorString += " in ${path}"
        }
        return errorString
    }
    return None
}

public func sourcePathCheck(path: Path): Bool {
    !Regex(REGEXP_COMMAND_INJECTION).matches(path.toString())
}

public func customizedOptionCheck(option: String, path: String): Bool {
    let hasPath: Bool = !path.isEmpty()
    if (option.size <= 1) {
        var errorString: String = "Error: the length of customized key must be greater than 1"
        if (hasPath) {
            errorString += " in ${path}"
        }
        eprintln(errorString)
        return false
    }
    var matched = Regex(REGEXP_CONDITON_VALUE)
    match (matched.find(option)) {
        case Some(_) => return true
        case None =>
            var errorString: String = "Error: the customized key '${option}'"
            if (hasPath) {
                errorString += " in ${path}"
            }
            errorString += " is wrong, the example is '${EXAMPLE_CONDITON_VALUE}'"
            eprintln(errorString)
    }
    return false
}

// Check dependencies 'path' field in cjpm.toml.
func depPathCheck(text: String, errLoc: String, modulePath: String, moduleName: String): Bool {
    if (text.isEmpty()) {
        eprintln("Error: the path can't be empty which is listed in ${errLoc}")
        return false
    }
    let depPath: String = getPath(modulePath, text)
    if (!directoryExists(depPath)) {
        eprintln("Error: the path '${text}' of module '${moduleName}' does not exist which is listed in ${errLoc}")
        return false
    }
    return true
}

public func srcDirCheck(text: String, path: String, configPath: String): Bool {
    if (!safeCheck(text, configPath)) {
        return false
    }
    let srcDirPath = getPath(path, text)
    if (!directoryExists(srcDirPath)) {
        eprintln("Error: path of src directory '${text}' in ${configPath} does not exist")
        return false
    }
    let curDirPath = getCanonicalPath(path)
    if (!srcDirPath.startsWith(curDirPath)) {
        eprintln("Error: src file directory ${text} must be contained in module directory. " +
            "Please check field 'src-dir' in ${configPath}")
        return false
    }
    return true
}

public func envNameCheck(envName: String, cmd: String, configFilePath: String): Bool {
    var matched = Regex(REGEXP_CONDITON_VALUE)
    match (matched.find(envName)) {
        case Some(_) => return true
        case None => eprintln("Error: environment name '${envName}' in profile '${cmd}' in ${configFilePath} " +
            "is wrong, the example is '${EXAMPLE_ENV_NAME}'")
    }
    return false
}

public func fixCompileOption(compileOption: String, configFilePath: String): String {
    var res = compileOption
    res = fixTestOption(res, configFilePath)
    return res
}

func fixTestOption(compileOption: String, configFilePath: String): String {
    var flag = true
    var res = compileOption
    if (res == "--test") {
        res = ""
        flag = false
    }
    if (res.contains(" --test ")) {
        res = res.replace(" --test ", " ")
        flag = false
    }
    if (res.startsWith("--test ")) {
        res = res.removePrefix("--test ")
        flag = false
    }
    if (res.endsWith(" --test")) {
        res = res.removeSuffix(" --test")
        flag = false
    }

    if (!flag) {
        eprintln("Warning: '--test' in compile option of '${configFilePath}' will be ignored, " +
            "test configuration will be automatically added by 'cjpm test'")
    }

    return res
}
