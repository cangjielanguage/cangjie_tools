// Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
// This source file is part of the Cangjie project, licensed under Apache-2.0
// with Runtime Library Exception.
//
// See https://cangjie-lang.cn/pages/LICENSE for license information.

package cjpm.implement

import std.collection.*
import std.fs.*
import std.regex.*
import stdx.serialization.serialization.*
import stdx.encoding.json.*
import stdx.compress.*
import cjpm.config.*

let README_FILE = "README.md"
let README_ZH_FILE = "README_zh.md"
let META_DATA_FILE = "meta-data.json"

/*
 * Make tarball of current module for publish
 *
 * STEPS:
 * 1. Verify configuration in cjpm.toml;
 * 2. Run 'cjpm build/test' to verify module;
 * 3. Run cjlint for source code;
 * 4. Make tarball, excluding specific files.
 */
public func doBundle(bundleConfig: BundleConfig): Bool {
    printBundle("cjpm bundle: start bundling...", bundleConfig.isVerbose)

    let tomlInfo: TomlInfo = loadModuleFile(DIR_CURRENT) ?? return false
    printBundle("cjpm bundle: checking related configuration of current module...", bundleConfig.isVerbose)

    // verify some configuration
    if (!verifyBundleToml(tomlInfo)) {
        return false
    }

    // make and check meta-data
    let metaData = MetaData(
        tomlInfo.pkg.organization,
        tomlInfo.pkg.name,
        tomlInfo.pkg.version,
        tomlInfo.pkg.description,
        "src",
        tomlInfo.pkg.outputType == OutputType.Exe,
        tomlInfo.pkg.authors,
        tomlInfo.pkg.repositroy,
        tomlInfo.pkg.homepage,
        tomlInfo.pkg.documentation,
        tomlInfo.pkg.tag,
        tomlInfo.pkg.category,
        tomlInfo.pkg.license,
        ArtifactIndex(
            tomlInfo.pkg.organization,
            tomlInfo.pkg.name,
            tomlInfo.pkg.version,
            ArrayList<IndexDep>(),
            ArrayList<IndexDep>(),
            ArrayList<IndexDep>(),
            "",
            false,
            tomlInfo.pkg.cjcVersion,
            INDEX_VERSION_BYTE
        ),
        META_DATA_VERSION_BYTE
    )
    if (!metaData.checkMetaData(tomlInfo)) {
        return false
    }

    // Verify build/test
    printBundle("cjpm bundle: checking build and test...", bundleConfig.isVerbose)
    if (!checkBuildTest(bundleConfig, tomlInfo)) {
        return false
    }

    // Verify cjlint rules
    printBundle("cjpm bundle: checking lint warnings...", bundleConfig.isVerbose)
    if (bundleConfig.isLint && !doLint(tomlInfo, bundleConfig.isVerbose)) {
        return false
    }

    // make tarball
    return makeTarball(bundleConfig, tomlInfo, metaData)
}

/*
 * Verify configuration in cjpm.toml
 *
 * 1. Workspace cannot be bundled and published;
 * 2. Readme.md is necessary;
 * 3. All types of dependencies should be in central repository.
 */
func verifyBundleToml(tomlInfo: TomlInfo): Bool {
    if (tomlInfo.isWorkSpaceConfig) {
        eprintln("Error: cannot bundle a workspace")
        return false
    }

    // check readme
    if (!verifyReadme()) {
        eprintln("Error: a valid artifact should contain a README.md or README_zh.md in root directory")
        return false
    }

    // check description
    if (tomlInfo.pkg.description.isEmpty()) {
        println("Error: a valid artifact should have a description in field 'package.description'")
        return false
    }

    return true
}

/*
 * Ensure that published module can be successfully built or tested
 *
 * 1. Run 'cjpm test', except for --skip-test;
 * 2. Run 'cjpm build -i' when --skip-test or when output-type is 'executable'.
 */
func checkBuildTest(bundleConfig: BundleConfig, tomlInfo: TomlInfo): Bool {
    // cjpm clean
    if (!doClean(DIR_CURRENT, "", false)) {
        return false
    }

    // cjpm test
    if (bundleConfig.isTest) {
        let testConfig = TestConfig()
        testConfig.isVerbose = bundleConfig.isVerbose
        if (!doTest(Array<String>(), testConfig)) {
            eprintln("Error: running 'cjpm test' failed")
            return false
        }
    }

    // cjpm build -i
    if (!bundleConfig.isTest || tomlInfo.pkg.outputType == OutputType.Exe) {
        let buildConfig = BuildConfig()
        buildConfig.isVerbose = bundleConfig.isVerbose
        buildConfig.isIncremental = true
        if (!doBuild(buildConfig)) {
            eprintln("Error: running 'cjpm build -i' failed")
            return false
        }
    }

    return true
}

/*
 * Make tar.gz of current module, and renamed as name.cjp
 *
 * 1. File list: files in 'include' and not in 'exclude';
 * 2. Make tarball;
 * 3. Save tarball and meta-data.
 */
func makeTarball(bundleConfig: BundleConfig, tomlInfo: TomlInfo, metaData: MetaData): Bool {
    bundleConfig.currentDir = getCanonicalPath(DIR_CURRENT)
    bundleConfig.targetDir = getOriginTargetDirectory(TARGET, DIR_CURRENT, "", tomlInfo.pkg.targetDir, true) ?? return false
    bundleConfig.scriptDir = getScriptDirectory(BUILD_SCRIPT_DIR, bundleConfig.currentDir, tomlInfo.pkg.scriptDir, false) ?? return false

    // make tarball
    printBundle("cjpm bundle: making tarball of source code...", bundleConfig.isVerbose)
    if (!compressCjp(bundleConfig, tomlInfo, metaData)) {
        return false
    }

    // write meta-data
    printBundle("cjpm bundle: making meta-data of source code...", bundleConfig.isVerbose)
    let metaDataJson = metaData.serialize().toJson().toJsonString()
    try {
        File.writeTo(Path(bundleConfig.targetDir).join(META_DATA_FILE), metaDataJson.toArray())
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: failed to make ${META_DATA_FILE}")
        return false
    }

    return true
}

/*
 * compress files in specific file list
 */
func compressCjp(bundleConfig: BundleConfig, tomlInfo: TomlInfo, metaData: MetaData): Bool {
    // get file list
    let fileSet = HashSet<String>()
    getBundleFileList(DIR_CURRENT, fileSet, bundleConfig, tomlInfo)

    // compress
    let tarPath = getPath(bundleConfig.targetDir, "${tomlInfo.pkg.name}-${tomlInfo.pkg.version}.cjp")
    let targetCopyDir = Path(bundleConfig.targetDir).join("${tomlInfo.pkg.name}-${tomlInfo.pkg.version}").toString()

    try {
        for (filePath in fileSet) {
            if (!createDirectory(targetCopyDir + Path(filePath).parent.toString().removePrefix(bundleConfig.currentDir))) {
                return false
            }
            copy(filePath, to: targetCopyDir + filePath.removePrefix(bundleConfig.currentDir), overwrite: true)
        }
        TarGzip.archive(
            fromDir: targetCopyDir,
            destFile: tarPath,
            includeBaseDirectory: true
        )
    } catch (e: Exception) {
        eprintln(e.message)
        eprintln("Error: failed to make tarball for current module")
        return false
    } finally {
        deleteDirectory(targetCopyDir)
    }

    // generate check sum
    if (!metaData.index.genFileCheckSum(tarPath)) {
        eprintln("Error: failed to generate check-sum of artifact package '${tarPath}")
        return false
    }

    return true
}

/*
 * Get file list that should be bundled. Conditions are:
 * 1. cjpm.toml and README.md will always be bundled;
 * 2. Binary files should not be bundled;
 * 3. Files matched with any rules in 'package.exclude' should not be bundled;
 * 4. Files matched with any rules in 'package.include' should be bundled; if 'package.include' is empty, all files will be matched;
 * 5. cjpm.lock, cangjie-repo.toml, target directory and script-cache directory should not be bundled.
 */
func getBundleFileList(dir: String, fileSet: HashSet<String>, bundleConfig: BundleConfig, tomlInfo: TomlInfo): Unit {
    // deal with files
    for (fileInfo in getFileList(dir)) {
        // binary check
        if (isBinary(fileInfo)) {
            continue
        }

        let fileCanonicalPath = getCanonicalPath(fileInfo.path.toString())
        let fileRelativePath = fileCanonicalPath.
            removePrefix(bundleConfig.currentDir).
            removePrefix(PATH_SPLIT_CHAR).
            replace("\\", "/")

        // cjpm.toml and README.md
        if ((fileRelativePath == CONFIG_FILE_NAME) || (fileRelativePath == README_FILE) || (fileRelativePath == README_ZH_FILE)) {
            fileSet.add(fileCanonicalPath)
            continue
        }

        // cjpm.lock && cangjie-repo.toml
        if ((fileRelativePath == LOCK_FILE_NAME) || (fileRelativePath == REPO_CONFIG_FILE_NAME)) {
            continue
        }

        // include check
        var isInclude = false
        for (rule in tomlInfo.pkg.includeRules) {
            if (isPathIncluded(fileRelativePath, rule)) {
                isInclude = true
                break
            }
        }
        if (!tomlInfo.pkg.includeRules.isEmpty() && !isInclude) {
            continue
        }

        // exclude check
        isInclude = true
        for (rule in tomlInfo.pkg.excludeRules) {
            if (isPathIncluded(fileRelativePath, rule)) {
                isInclude = false
                break
            }
        }
        if (!isInclude) {
            continue
        }

        // file is included
        fileSet.add(fileCanonicalPath)
    }

    // deal with directories
    for (dirInfo in getDirectoryList(dir)) {
        let dirPath = getCanonicalPath(dirInfo.path.toString())
        if ((dirPath == bundleConfig.targetDir) || (dirPath == bundleConfig.scriptDir)) {
            continue
        }
        getBundleFileList(dirPath, fileSet, bundleConfig, tomlInfo)
    }
}

/*
 * Verify binary files
 * Judgement: files with letter '0x00' in it
 */
func isBinary(fileInfo: FileInfo): Bool {
    try {
        let fileArr: Array<Byte> = File.readFrom(fileInfo.path)
        return fileArr.contains(0x00)
    } catch (_: Exception) {
        return true
    }
}

/*
 * Verify if a file path match a rule for include/exclude
 * 'path' should always be a file, and has already been transfered into relative path, with '\' replaced by '/'
 */
func isPathIncluded(path: String, rule: String): Bool {
    if (rule.isEmpty() || rule.startsWith("#")) {
        return false
    }

    var regexRule = ""
    var processedpattern = rule.trimAscii()

    // process rules start with '/'
    if (processedpattern.startsWith("/")) {
        regexRule = "^"
        processedpattern = processedpattern[1..]
    } else if (processedpattern.startsWith("./")) {
        regexRule = "^"
        while (processedpattern.startsWith("./")) {
            processedpattern.removePrefix("./")
        }
    } else {
        regexRule = "^(.*/)?"
    }

    // start running pattern-process
    var i = 0
    while (i < processedpattern.size) {
        let ch = Rune(processedpattern[i])

        match (ch) {
            case r'*' =>
                if ((i + 1 < processedpattern.size) && (Rune(processedpattern[i + 1]) == r'*')) {
                    // deal with '**'
                    if ((i + 2 < processedpattern.size) && (Rune(processedpattern[i + 2]) == r'/')) {
                        // deal with '/**/'
                        regexRule += "(.*/)?"
                        i += 2
                    } else {
                        // deal with '**/'
                        regexRule += ".*"
                        i += 1
                    }
                } else {
                    regexRule += "[^/]*"
                }
            case r'?' =>
                regexRule += "[^/]"
            case r'[' =>
                while ((i < processedpattern.size) && Rune(processedpattern[i]) != r']') {
                    regexRule += "${Rune(processedpattern[i])}"
                    i += 1
                }
                if (i < processedpattern.size) {
                    regexRule += "${Rune(processedpattern[i])}"
                }
            case r'.' => regexRule += "\\${ch}"
            case r'+' => regexRule += "\\${ch}"
            case r'^' => regexRule += "\\${ch}"
            case r'$' => regexRule += "\\${ch}"
            case r'|' => regexRule += "\\${ch}"
            case r'{' => regexRule += "\\${ch}"
            case r'}' => regexRule += "\\${ch}"
            case r'(' => regexRule += "\\${ch}"
            case r')' => regexRule += "\\${ch}"
            case r'\\' => regexRule += "\\${ch}"
            case _ =>
                regexRule += "${ch}"
        }

        i += 1
    }

    if (processedpattern.endsWith("/")) {
        regexRule += ".*$"
    } else {
        regexRule += "(/.*)?$"
    }

    return Regex(regexRule).matches(path)
}

func printBundle(logStr: String, isVerbose: Bool): Unit {
    if (isVerbose) {
        println(logStr)
    }
}

@When [os != "Windows"]
func verifyReadme(): Bool {
    return fileExists(getPath(DIR_CURRENT, README_FILE)) || fileExists(getPath(DIR_CURRENT, README_ZH_FILE))
}

@When [os == "Windows"]
func verifyReadme(): Bool {
    let fileList = getFileList(DIR_CURRENT)
    for (fileInfo in fileList) {
        let filePath = getCanonicalPath(fileInfo.path.toString())
        if (filePath.endsWith("${PATH_SPLIT_CHAR}${README_FILE}") || filePath.endsWith("${PATH_SPLIT_CHAR}${README_ZH_FILE}")) {
            return true
        }
    }
    return false
}