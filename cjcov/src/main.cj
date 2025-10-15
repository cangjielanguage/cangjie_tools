// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjcov

import std.collection.*
import std.process.*
import std.fs.*
import std.regex.*
import cjcov.util.*
import cjcov.core.*

// Support the include and exclude path in the form of regular expressions with '*'.
let PATH_REGEX_PATTERN = #"^[-\\/._a-zA-Z0-9]*\*.*$"#

func clearJunkFiles(path: String, rootArr: ArrayList<String>): Unit {
    deleteFileWithSuffix(path, ".html", false)
    for (rootPath in rootArr) {
        deleteFileWithSuffix(rootPath, ".gcov", true)
    }

    return
}

public func checkIncludeExcludeParams(params: ArrayList<String>): ArrayList<String> {
    var res: ArrayList<String> = ArrayList<String>()
    var curPath = toCanonical(".")
    for (param in params) {
        var path = param.removeSuffix(getSep())
        path = convertPath(path)
        match (Regex(PATH_REGEX_PATTERN).matcher(path).fullMatch()) {
            case Some(re) =>
                var sep = getSep()

                // -e ".//dir1/*_test.cj" will be converted to -e 'currentAbspath/dir1/.*_test\.cj'
                // -e ".\\dir1\*_test.cj" will be converted to -e 'currentAbspath\dir1\.*_test\.cj'
                res.add(re.matchString().replace("." + sep, curPath).replace(sep + sep, sep))
                continue
            case None => ()
        }

        if (exists(path)) {
            path = toCanonical(path)
        }

        // If the path contains a regular expression, the path does not need to be processed as an absolute path.
        res.add(path)
    }

    return res
}

func checkRootOption(options: Command, curPath: String): Bool {
    var rootArrSize = options.rootArr.size
    if (rootArrSize == 0) {
        options.rootArr.add(curPath)
    } else {
        for (i in (0..rootArrSize)) {
            var root = options.rootArr[i].removeSuffix(getSep())
            root = convertPath(root)
            if (!directoryExists(root)) {
                eprintln("path: ${root} does not exist.")
                return false
            }
            options.rootArr[i] = toCanonical(root)
        }
    }

    return true
}

func checkSourceOption(options: Command): Bool {
    var sourceArrSize = options.sourceArr.size
    for (i in (0..sourceArrSize)) {
        var source = options.sourceArr[i].removeSuffix(getSep())
        source = convertPath(source)
        if (!directoryExists(source)) {
            eprintln("path: ${source} does not exist.")
            return false
        }
        options.sourceArr[i] = toCanonical(source)
    }

    return true
}

func checkIncludeAndExclude(options: Command): Bool {
    options.excludeArr = checkIncludeExcludeParams(options.excludeArr)
    options.includeArr = checkIncludeExcludeParams(options.includeArr)

    var duplicateElements = getDuplicateElements(options.excludeArr, options.includeArr)
    if (duplicateElements.size != 0) {
        eprintln("--exclude and --include have duplicate paths, duplicate elements: ${duplicateElements}")
        return false
    }

    return true
}

func checkOutputOption(options: Command, curPath: String): Bool {
    // If the output path is not specified, it defaults to the current path, path is converted to absolute path finally.
    var optionOutput = options.output.removeSuffix(getSep())
    optionOutput = convertPath(optionOutput)
    if (optionOutput.size == 0) {
        options.output = curPath
    } else {
        if (!directoryExists(optionOutput)) {
            // If user specify the output path, but the parent path does not exist, return an error, otherwise create the specified output path
            var optionOutputPath: Path = if (Path(optionOutput).isRelative()) {
                Path(curPath).join(optionOutput)
            } else {
                Path(optionOutput)
            }
            let errorInfo = "The directory " + optionOutput +
                " does not have parent directory, the special directory such as '/' is supposed to have no parent directory."
            var parent = optionOutputPath.parent
            if (parent.isEmpty()) {
                throw FSException(errorInfo)
            }
            if (!directoryExists(parent.toString())) {
                eprintln("parent path: ${parent} which is assigned by --output does not exist.")
                return false
            }
            try {
                Directory.create(optionOutput)
            } catch (e: FSException) {
                eprintln("create outputpath: ${optionOutput} failed.\nException: ${e.toString()}")
                return false
            }
        }

        options.output = toCanonical(optionOutput)
    }

    return true
}

public func initCmdOptions(options: Command): (Bool, Command) {
    var curPath = toCanonical(".")

    if (!checkRootOption(options, curPath) || !checkSourceOption(options) || !checkIncludeAndExclude(options) ||
        !checkOutputOption(options, curPath)) {
        return (false, options)
    }

    clearJunkFiles(options.output, options.rootArr)

    var logDir = options.output + getSep() + "cjcov_logs"
    if (directoryExists(logDir)) {
        remove(logDir, recursive: true)
    }

    try {
        Directory.create(logDir)
    } catch (e: FSException) {
        eprintln("create log directory: ${logDir} failed.\nException: ${e.toString()}")
        return (false, options)
    }
    return (true, options)
}

@When[os == "Windows"]
func setCjcovEnv() {
    let cangjieHome = Process.current.getEnv("CANGJIE_HOME") ?? ""
    let llvmBin = cangjieHome + #"\third_party\llvm\bin"#
    let llvmLib = cangjieHome + #"\third_party\llvm\lib"#
    if (directoryExists(llvmBin)) {
        modifyEnv("PATH", llvmBin, separator: ";")
    }
}

@When[os == "Linux" || os == "macOS"]
func setCjcovEnv() {
    let cangjieHome = Process.current.getEnv("CANGJIE_HOME") ?? ""
    let llvmBin = cangjieHome + "/third_party/llvm/bin"
    let llvmLib = cangjieHome + "/third_party/llvm/lib"
    if (directoryExists(llvmBin)) {
        modifyEnv("PATH", llvmBin)
    }

    if (directoryExists(llvmLib)) {
        modifyEnv("LD_LIBRARY_PATH", llvmLib)
    }
}

func covBuild(options: Command): Bool {
    var ret: Bool = true

    ret = generateGcovFiles(options)
    if (!ret) {
        // Error info has been reported before.
        return false
    }

    var allGcovFiles: HashSet<String> = HashSet<String>()
    for (rootPath in options.rootArr) {
        allGcovFiles.add(all: HashSet<String>(getAllFilesWithSuffix(rootPath, ".gcov", true)))
    }

    if (allGcovFiles.size == 0) {
        println("There are no *.gcov files, coverage reports will not be generated.")
        return false
    }

    var mergeGcovData = processAllGcov(allGcovFiles, options)

    var covdata: HashMap<String, CoverageData> = HashMap<String, CoverageData>()
    for ((srcFileName, data) in mergeGcovData) {
        printCoverageData(options, srcFileName, data)
        covdata[srcFileName] = CoverageData(srcFileName, data)
    }

    if (covdata.size == 0) {
        eprintln("generate html failed.")
        return false
    }

    if (options.xml) {
        genXmlReport(covdata, options)
    }

    if (options.json) {
        genJsonReport(covdata, options)
    }

    ret = genHtmlReport(covdata, options)
    if (!ret) {
        eprintln("generate html failed.")
        return false
    }

    println("generate html success.")

    if (!options.keep) {
        ret = clearFiles(allGcovFiles.toArray())
        if (!ret) {
            eprintln("delete gcov files: ${allGcovFiles} failed.")
            return false
        }
    }

    return true
}

@When[backend == "cjnative"]
func checkLLVMCov(): Int64 {
    setCjcovEnv()
    let (flag, _, _) = subprocessCallWithOutput("llvm-cov", ArrayList<String>.of("--version"))
    if (!flag) {
        eprintln("llvm-cov not found, please set CANGJIE_HOME or install llvm.")
        return 1
    }
    return 0
}

main(): Int64 {
    let sysArgs = Process.current.arguments
    var argMap: HashMap<String, String> = HashMap<String, String>()

    // If only one system parameter gived, return a default Command class.
    // Cjlint(G.CHK.01) can not check cross-function, so put the size check of 'sysArgs' here.
    if (sysArgs.size > 0) {
        argMap = getParseOption(sysArgs)

        // If get a input like this: "cjcov abcxxxse", then print the simple usage.
        // If just get a input line this: "cjcov", the program should keep running.
        if (argMap.size == 0) {
            print(getSimpleUsage())
            return 1
        }
    }

    var optionsOrigin = Command(argMap)

    // Refer to the linux grep command display format, -v|--version has the highest priority, -h|--help has the next highest priority.
    if (optionsOrigin.version) {
        println(getCjVersion())
        return 0
    }

    if (optionsOrigin.help) {
        print(getCjcovHelp())
        return 0
    }

    if (checkLLVMCov() != 0) {
        return 1
    }

    var (ret, options) = initCmdOptions(optionsOrigin)
    if (!ret) {
        eprintln("parse cmd failed, input args: ${sysArgs} ")
        return 1
    }

    if (!covBuild(options)) {
        return 1
    }

    return 0
}
